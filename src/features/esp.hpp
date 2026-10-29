#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include "../core/math.hpp"

namespace Features::ESP {

    struct PhaseInfo {
        std::string name;
        uint32_t color = 0;
    };

    PhaseInfo GetNeighborPhase(uintptr_t sosedActor);
    bool IsKeyQuestItem(const std::string& name);
    std::string FormatItemName(const std::string& rawName);

    void Render(
        const Core::Math::Vector3& camLoc,
        const Core::Math::Rotator& camRot,
        float camFov,
        float screenW,
        float screenH,
        const std::vector<uintptr_t>& actors,
        uintptr_t localPawn
    );

}
