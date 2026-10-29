#pragma once
#include <cstdint>
#include "../core/math.hpp"

namespace Features::Noclip {

    void Update(
        uintptr_t localPawn,
        uintptr_t uworld,
        const Core::Math::Vector3& camLoc,
        const Core::Math::Rotator& camRot
    );

}
