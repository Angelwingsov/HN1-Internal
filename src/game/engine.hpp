#pragma once
#include <vector>
#include <cstdint>
#include "../core/math.hpp"

namespace Game::Engine {

    uintptr_t GetWorld();
    uintptr_t GetPlayerController();
    uintptr_t GetLocalPawn();
    uintptr_t GetCameraManager();

    bool GetCameraView(
        Core::Math::Vector3& outLoc,
        Core::Math::Rotator& outRot,
        float& outFov
    );

    uintptr_t GetPersistentLevel();
    bool GetLevelActors(std::vector<uintptr_t>& outActors);

    void SetMouseInput(bool bShowCursor);

}
