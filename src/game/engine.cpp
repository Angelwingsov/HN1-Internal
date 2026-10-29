#include "engine.hpp"
#include "offsets.hpp"
#include "ue_types.hpp"
#include "../core/memory.hpp"

namespace Game::Engine {

    using namespace Core::Memory;

    uintptr_t GetWorld() {
        const uintptr_t base = Core::Memory::GetModuleBase();
        if (!base)
            return 0;

        return SafeRead<uintptr_t>(base + Offsets::Engine::GWorld);
    }

    uintptr_t GetPlayerController() {
        const uintptr_t world = GetWorld();
        if (!world)
            return 0;

        const uintptr_t gameInstance = SafeRead<uintptr_t>(world + Offsets::UWorld::GameInstance);
        if (!gameInstance)
            return 0;

        const uintptr_t localPlayersArray = SafeRead<uintptr_t>(gameInstance + Offsets::GameInstance::LocalPlayers);
        if (!localPlayersArray)
            return 0;

        const uintptr_t localPlayer = SafeRead<uintptr_t>(localPlayersArray);
        if (!localPlayer)
            return 0;

        return SafeRead<uintptr_t>(localPlayer + Offsets::LocalPlayer::PlayerController);
    }

    uintptr_t GetLocalPawn() {
        const uintptr_t pc = GetPlayerController();
        if (!pc)
            return 0;

        uintptr_t pawn = SafeRead<uintptr_t>(pc + Offsets::PlayerController::PawnPrimary);
        if (!pawn)
            pawn = SafeRead<uintptr_t>(pc + Offsets::PlayerController::PawnFallback1);
        if (!pawn)
            pawn = SafeRead<uintptr_t>(pc + Offsets::PlayerController::PawnFallback2);

        return pawn;
    }

    uintptr_t GetCameraManager() {
        const uintptr_t pc = GetPlayerController();
        if (!pc)
            return 0;

        return SafeRead<uintptr_t>(pc + Offsets::PlayerController::CameraManager);
    }

    bool GetCameraView(
        Core::Math::Vector3& outLoc,
        Core::Math::Rotator& outRot,
        float& outFov
    ) {
        const uintptr_t camMgr = GetCameraManager();
        if (!camMgr)
            return false;

        outLoc = SafeRead<Core::Math::Vector3>(camMgr + Offsets::CameraManager::CameraLocation);
        outRot = SafeRead<Core::Math::Rotator>(camMgr + Offsets::CameraManager::CameraRotation);
        outFov = SafeRead<float>(camMgr + Offsets::CameraManager::CameraFOV, 90.0f);

        if (outFov < 10.0f || outFov > 170.0f)
            outFov = 90.0f;

        return true;
    }

    uintptr_t GetPersistentLevel() {
        const uintptr_t world = GetWorld();
        if (!world)
            return 0;

        return SafeRead<uintptr_t>(world + Offsets::UWorld::PersistentLevel);
    }

    bool GetLevelActors(std::vector<uintptr_t>& outActors) {
        const uintptr_t level = GetPersistentLevel();
        if (!level)
            return false;

        const uintptr_t actorsData = SafeRead<uintptr_t>(level + Offsets::Level::Actors);
        const int32_t count = SafeRead<int32_t>(level + Offsets::Level::ActorCount);

        if (!actorsData || count <= 0 || count > Offsets::Constants::MaxLevelActorsLimit)
            return false;

        outActors.clear();
        outActors.reserve(count);

        for (int32_t i = 0; i < count; ++i) {
            const uintptr_t actor = SafeRead<uintptr_t>(actorsData + (i * sizeof(void*)));
            if (actor)
                outActors.push_back(actor);
        }

        return true;
    }

    void SetMouseInput(bool bShowCursor) {
        const uintptr_t pc = GetPlayerController();
        if (!pc)
            return;

        const uint8_t flags = SafeRead<uint8_t>(pc + Offsets::PlayerController::MouseFlags);

        if (bShowCursor) {
            SafeWrite<uint8_t>(pc + Offsets::PlayerController::MouseFlags, flags | UE::Flags::MouseAllInput);
        } else {
            SafeWrite<uint8_t>(pc + Offsets::PlayerController::MouseFlags, flags & ~UE::Flags::MouseAllInput);
        }
    }

}
