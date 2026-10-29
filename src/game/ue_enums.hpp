#pragma once
#include <cstdint>
#include "../core/math.hpp"

namespace Game::UE {

    enum class EMovementMode : uint8_t {
        MOVE_None        = 0,
        MOVE_Walking     = 1,
        MOVE_NavWalking  = 2,
        MOVE_Falling     = 3,
        MOVE_Swimming    = 4,
        MOVE_Flying      = 5,
        MOVE_Custom      = 6,
        MOVE_MAX         = 7
    };

    enum class ECollisionEnabled : uint8_t {
        NoCollision      = 0,
        QueryOnly        = 1,
        PhysicsOnly      = 2,
        QueryAndPhysics  = 3,
        MAX              = 4
    };

    enum class ENeighborState : uint8_t {
        NS_IDLE          = 0,
        NS_HUNT          = 1,
        NS_ATTACK        = 2,
        NS_DEMO          = 3,
        NS_TUTORIAL      = 4,
        NS_MAX           = 5
    };

    namespace Flags {
        constexpr uint8_t MouseCursor           = 0x01;
        constexpr uint8_t MouseClickEvents      = 0x02;
        constexpr uint8_t MouseOverEvents       = 0x08;
        constexpr uint8_t MouseAllInput         = MouseCursor | MouseClickEvents | MouseOverEvents;

        constexpr uint8_t ActorEnableCollision  = 0x01;
        constexpr uint8_t OwnerNoSee            = (1 << 2);
        constexpr uint8_t DoCollisionTest       = 0x01;
        constexpr uint8_t DisableGravity        = (1 << 7);
        constexpr uint8_t CheatFlying           = (1 << 2);
        constexpr uint8_t WorldBoundsCheck      = 0x01;
    }

    struct FTeleportParams {
        Core::Math::Vector3 DestLocation;
        Core::Math::Rotator DestRotation;
        bool ReturnValue = false;
        uint8_t Pad[3]   = { 0 };
    };

    struct FSetMovementModeParams {
        uint8_t NewMovementMode = 0;
        uint8_t NewCustomMode   = 0;
    };

    struct FHeadData {
        float YellowDistance = 0.0f;
        float RedDistance    = 0.0f;
        float LoudDistance   = 0.0f;
        float SilentDistance = 0.0f;
    };

}
