#include "noclip.hpp"
#include "../config/settings.hpp"
#include "../core/memory.hpp"
#include "../game/offsets.hpp"
#include "../game/ue_types.hpp"
#include "../game/ue_enums.hpp"

namespace Features::Noclip {

    using namespace Core::Math;
    using namespace Core::Memory;
    using namespace Game;

    void Update(
        uintptr_t localPawn,
        uintptr_t uworld,
        const Vector3& camLoc,
        const Rotator& camRot
    ) {
        if (!IsValidPtr(localPawn))
            return;

        const uintptr_t pawn = localPawn;

        uintptr_t capsule = SafeRead<uintptr_t>(pawn + Offsets::Human::CapsuleComponent);
        if (!capsule) capsule = SafeRead<uintptr_t>(pawn + Offsets::Human::CapsuleFallback);

        const uintptr_t moveComp = SafeRead<uintptr_t>(pawn + Offsets::Human::MovementComponent);
        uintptr_t rootComp = SafeRead<uintptr_t>(pawn + Offsets::Human::RootComponent);
        if (!rootComp && capsule)
            rootComp = capsule;

        if (!rootComp)
            return;

        static uintptr_t s_fnTeleport = 0;
        static uintptr_t s_fnSetCol = 0;
        static uintptr_t s_fnSetMode = 0;

        if (!s_fnTeleport) {
            const uintptr_t pawnClass = SafeRead<uintptr_t>(pawn + Offsets::UObject::Class);
            s_fnTeleport = UE::FindFunctionFast(pawnClass, "K2_TeleportTo");
            s_fnSetCol = UE::FindFunctionFast(pawnClass, "SetActorEnableCollision");
            if (moveComp) {
                const uintptr_t moveClass = SafeRead<uintptr_t>(moveComp + Offsets::UObject::Class);
                s_fnSetMode = UE::FindFunctionFast(moveClass, "SetMovementMode");
            }
        }

        if (uworld) {
            const uintptr_t worldSettings = SafeRead<uintptr_t>(uworld + Offsets::UWorld::WorldSettings);
            if (worldSettings) {
                SafeWrite<float>(worldSettings + Offsets::WorldSettings::KillZ, Offsets::Constants::KillZBypassValue);
                const uint8_t boundsBit = SafeRead<uint8_t>(worldSettings + Offsets::WorldSettings::WorldBoundsChecks);
                SafeWrite<uint8_t>(worldSettings + Offsets::WorldSettings::WorldBoundsChecks, boundsBit & ~UE::Flags::WorldBoundsCheck);
            }
        }

        const auto& settings = Config::Get().movement;

        if (settings.noclip) {
            if (moveComp) {
                SafeWrite<uint8_t>(moveComp + Offsets::Movement::MovementMode, static_cast<uint8_t>(UE::EMovementMode::MOVE_Flying));
                SafeWrite<float>(moveComp + Offsets::Movement::GravityScale, 0.0f);
                SafeWrite<Vector3>(moveComp + Offsets::Movement::Velocity, Vector3(0.0f, 0.0f, 0.0f));
                SafeWrite<uint8_t>(moveComp + Offsets::Movement::CheatFlying, UE::Flags::CheatFlying);
                SafeWrite<float>(moveComp + Offsets::Movement::MaxFlySpeed, 100000.0f);
                SafeWrite<float>(moveComp + Offsets::Movement::BrakingDecelerationFlying, 0.0f);
            }

            const uint8_t gravBit = SafeRead<uint8_t>(pawn + Offsets::Human::GravityFlags);
            SafeWrite<uint8_t>(pawn + Offsets::Human::GravityFlags, gravBit | UE::Flags::DisableGravity);

            SafeWrite<uint8_t>(pawn + Offsets::Human::BlockFly, 0);

            if (capsule) {
                SafeWrite<uint8_t>(capsule + Offsets::SceneComponent::CollisionEnabled, static_cast<uint8_t>(UE::ECollisionEnabled::NoCollision));
            }
            const uint8_t actFlags = SafeRead<uint8_t>(pawn + Offsets::Human::ActorFlags);
            SafeWrite<uint8_t>(pawn + Offsets::Human::ActorFlags, actFlags & ~UE::Flags::ActorEnableCollision);

            if (s_fnSetCol) {
                bool bEnable = false;
                UE::SafeProcessEvent(pawn, s_fnSetCol, &bEnable);
            }

            const Vector3 curPos = SafeRead<Vector3>(rootComp + Offsets::SceneComponent::RelativeLocation);
            SafeWrite<Vector3>(pawn + Offsets::Human::PosGrave, curPos);

            Vector3 vForward, vRight, vUp;
            GetAxes(camRot, vForward, vRight, vUp);

            Vector3 moveDir(0.0f, 0.0f, 0.0f);
            float speed = settings.noclipSpeed;
            if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
                speed *= Offsets::Constants::NoclipBoostMultiplier;

            if (GetAsyncKeyState('W') & 0x8000) moveDir += vForward;
            if (GetAsyncKeyState('S') & 0x8000) moveDir -= vForward;
            if (GetAsyncKeyState('D') & 0x8000) moveDir += vRight;
            if (GetAsyncKeyState('A') & 0x8000) moveDir -= vRight;
            if (GetAsyncKeyState(VK_SPACE) & 0x8000) moveDir.Z += 1.0f;
            if (GetAsyncKeyState(VK_CONTROL) & 0x8000) moveDir.Z -= 1.0f;

            if (!moveDir.IsZero()) {
                const Vector3 currentLoc = SafeRead<Vector3>(rootComp + Offsets::SceneComponent::RelativeLocation);
                const Vector3 newLoc = currentLoc + (moveDir * speed);

                UE::FTeleportParams teleportParams;
                teleportParams.DestLocation = newLoc;
                teleportParams.DestRotation = camRot;
                teleportParams.ReturnValue = false;

                if (s_fnTeleport) {
                    UE::SafeProcessEvent(pawn, s_fnTeleport, &teleportParams);
                }

                SafeWrite<Vector3>(rootComp + Offsets::SceneComponent::RelativeLocation, newLoc);
                SafeWrite<Vector3>(pawn + Offsets::Human::PosGrave, newLoc);

                if (moveComp) {
                    SafeWrite<Vector3>(moveComp + Offsets::Movement::Velocity, Vector3(0.0f, 0.0f, 0.0f));
                }
            }
        } else {
            if (capsule) {
                const uint8_t col = SafeRead<uint8_t>(capsule + Offsets::SceneComponent::CollisionEnabled);
                if (col == static_cast<uint8_t>(UE::ECollisionEnabled::NoCollision)) {
                    SafeWrite<uint8_t>(capsule + Offsets::SceneComponent::CollisionEnabled, static_cast<uint8_t>(UE::ECollisionEnabled::QueryAndPhysics));
                    const uint8_t actFlags = SafeRead<uint8_t>(pawn + Offsets::Human::ActorFlags);
                    SafeWrite<uint8_t>(pawn + Offsets::Human::ActorFlags, actFlags | UE::Flags::ActorEnableCollision);

                    if (s_fnSetCol) {
                        bool bEnable = true;
                        UE::SafeProcessEvent(pawn, s_fnSetCol, &bEnable);
                    }

                    if (moveComp) {
                        SafeWrite<uint8_t>(moveComp + Offsets::Movement::MovementMode, static_cast<uint8_t>(UE::EMovementMode::MOVE_Walking));
                        SafeWrite<float>(moveComp + Offsets::Movement::GravityScale, 1.0f);
                        if (s_fnSetMode) {
                            UE::FSetMovementModeParams modeParams;
                            modeParams.NewMovementMode = static_cast<uint8_t>(UE::EMovementMode::MOVE_Walking);
                            modeParams.NewCustomMode = 0;
                            UE::SafeProcessEvent(moveComp, s_fnSetMode, &modeParams);
                        }
                    }

                    const uint8_t gravBit = SafeRead<uint8_t>(pawn + Offsets::Human::GravityFlags);
                    SafeWrite<uint8_t>(pawn + Offsets::Human::GravityFlags, gravBit & ~UE::Flags::DisableGravity);
                }
            }
        }
    }

}
