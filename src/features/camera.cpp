#include "camera.hpp"
#include "../config/settings.hpp"
#include "../core/memory.hpp"
#include "../core/math.hpp"
#include "../game/offsets.hpp"
#include "../game/ue_types.hpp"
#include "../game/ue_enums.hpp"

namespace Features::Camera {

    using namespace Core::Math;
    using namespace Core::Memory;
    using namespace Game;

    struct CameraBackup {
        float armLength = 0.0f;
        Vector3 socketOffset{};
        Vector3 targetOffset{};
        uint8_t armFlags = 0;
        uint8_t meshFlags = 0;
        bool isArmSaved = false;

        float camMgrFov = 90.0f;
        float camCompFov = 90.0f;
        bool isFovSaved = false;
    };

    static CameraBackup s_backup;
    static bool s_wasCustomFov = false;
    static bool s_wasThirdPerson = false;

    void Update(uintptr_t localPawn, uintptr_t cameraManager) {
        if (!IsValidPtr(localPawn))
            return;

        const auto& settings = Config::Get().camera;

        static uintptr_t s_fnSetOwnerNoSee = 0;
        static uintptr_t s_fnSetFieldOfView = 0;

        const uintptr_t camComp = SafeRead<uintptr_t>(localPawn + Offsets::Human::CameraComponent);

        if (cameraManager && !s_backup.isFovSaved) {
            s_backup.camMgrFov = SafeRead<float>(cameraManager + Offsets::CameraManager::DefaultFOV);
            if (camComp) {
                s_backup.camCompFov = SafeRead<float>(camComp + Offsets::CameraComponent::FieldOfView);
            }
            if (s_backup.camMgrFov > 10.0f && s_backup.camMgrFov < 170.0f) {
                s_backup.isFovSaved = true;
            }
        }

        if (settings.customFov) {
            SafeWrite<float>(cameraManager + Offsets::CameraManager::DefaultFOV, settings.customFovValue);
            if (camComp) {
                SafeWrite<float>(camComp + Offsets::CameraComponent::FieldOfView, settings.customFovValue);
                if (!s_fnSetFieldOfView) {
                    const uintptr_t camClass = SafeRead<uintptr_t>(camComp + Offsets::UObject::Class);
                    s_fnSetFieldOfView = UE::FindFunctionFast(camClass, "SetFieldOfView");
                }
                if (s_fnSetFieldOfView) {
                    float fov = settings.customFovValue;
                    UE::SafeProcessEvent(camComp, s_fnSetFieldOfView, &fov);
                }
            }
            s_wasCustomFov = true;
        } else if (s_wasCustomFov) {
            SafeWrite<float>(cameraManager + Offsets::CameraManager::DefaultFOV, s_backup.camMgrFov);
            if (camComp) {
                SafeWrite<float>(camComp + Offsets::CameraComponent::FieldOfView, s_backup.camCompFov);
                if (s_fnSetFieldOfView) {
                    float fov = s_backup.camCompFov;
                    UE::SafeProcessEvent(camComp, s_fnSetFieldOfView, &fov);
                }
            }
            s_wasCustomFov = false;
        }

        const uintptr_t springArm = SafeRead<uintptr_t>(localPawn + Offsets::Human::SpringArm);
        const uintptr_t mesh = SafeRead<uintptr_t>(localPawn + Offsets::Human::MeshComponent);

        if (springArm) {
            if (!s_backup.isArmSaved) {
                s_backup.armLength = SafeRead<float>(springArm + Offsets::SpringArm::TargetArmLength);
                s_backup.socketOffset = SafeRead<Vector3>(springArm + Offsets::SpringArm::SocketOffset);
                s_backup.targetOffset = SafeRead<Vector3>(springArm + Offsets::SpringArm::TargetOffset);
                s_backup.armFlags = SafeRead<uint8_t>(springArm + Offsets::SpringArm::CollisionFlags);
                if (mesh) {
                    s_backup.meshFlags = SafeRead<uint8_t>(mesh + Offsets::SceneComponent::OwnerNoSee);
                }
                s_backup.isArmSaved = true;
            }

            if (settings.thirdPerson) {
                SafeWrite<float>(springArm + Offsets::SpringArm::TargetArmLength, settings.thirdPersonDist);
                SafeWrite<Vector3>(
                    springArm + Offsets::SpringArm::SocketOffset,
                    Vector3(0.0f, settings.thirdPersonOffsetY, settings.thirdPersonOffsetZ)
                );

                const uint8_t armFlags = SafeRead<uint8_t>(springArm + Offsets::SpringArm::CollisionFlags);
                SafeWrite<uint8_t>(springArm + Offsets::SpringArm::CollisionFlags, armFlags | UE::Flags::DoCollisionTest);

                if (mesh) {
                    const uint8_t primFlags = SafeRead<uint8_t>(mesh + Offsets::SceneComponent::OwnerNoSee);
                    SafeWrite<uint8_t>(mesh + Offsets::SceneComponent::OwnerNoSee, primFlags & ~UE::Flags::OwnerNoSee);

                    if (!s_fnSetOwnerNoSee) {
                        const uintptr_t meshClass = SafeRead<uintptr_t>(mesh + Offsets::UObject::Class);
                        s_fnSetOwnerNoSee = UE::FindFunctionFast(meshClass, "SetOwnerNoSee");
                    }
                    if (s_fnSetOwnerNoSee) {
                        bool bNoSee = false;
                        UE::SafeProcessEvent(mesh, s_fnSetOwnerNoSee, &bNoSee);
                    }
                }
                s_wasThirdPerson = true;
            } else if (s_wasThirdPerson) {
                SafeWrite<float>(springArm + Offsets::SpringArm::TargetArmLength, s_backup.armLength);
                SafeWrite<Vector3>(springArm + Offsets::SpringArm::SocketOffset, s_backup.socketOffset);
                SafeWrite<Vector3>(springArm + Offsets::SpringArm::TargetOffset, s_backup.targetOffset);
                SafeWrite<uint8_t>(springArm + Offsets::SpringArm::CollisionFlags, s_backup.armFlags);

                if (mesh) {
                    SafeWrite<uint8_t>(mesh + Offsets::SceneComponent::OwnerNoSee, s_backup.meshFlags);
                    if (s_fnSetOwnerNoSee) {
                        bool bNoSee = ((s_backup.meshFlags & UE::Flags::OwnerNoSee) != 0);
                        UE::SafeProcessEvent(mesh, s_fnSetOwnerNoSee, &bNoSee);
                    }
                }
                s_wasThirdPerson = false;
            }
        }
    }

}
