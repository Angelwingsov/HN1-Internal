#pragma once
#include <cstdint>

namespace Game::Offsets {

    namespace Engine {
        constexpr uintptr_t GWorld               = 0x031DEB80;
        constexpr uintptr_t AppendString         = 0x006A50C0;
        constexpr size_t ProcessEventVTableIndex = 0x41;
    }

    namespace Constants {
        constexpr float KillZBypassValue            = -99999999.0f;
        constexpr float DefaultCapsuleHalfHeight    = 96.0f;
        constexpr float BoxWidthRatio               = 0.45f;
        constexpr int32_t MaxLevelActorsLimit       = 8192;
        constexpr float NoclipBoostMultiplier       = 2.5f;
    }

    namespace UWorld {
        constexpr uintptr_t PersistentLevel = 0x0030;
        constexpr uintptr_t GameInstance    = 0x0190;
        constexpr uintptr_t WorldSettings   = 0x0248;
    }

    namespace WorldSettings {
        constexpr uintptr_t KillZ             = 0x0340;
        constexpr uintptr_t WorldBoundsChecks = 0x0330;
    }

    namespace GameInstance {
        constexpr uintptr_t LocalPlayers = 0x0038;
    }

    namespace LocalPlayer {
        constexpr uintptr_t PlayerController = 0x0030;
    }

    namespace PlayerController {
        constexpr uintptr_t PawnPrimary   = 0x03B0;
        constexpr uintptr_t PawnFallback1 = 0x0368;
        constexpr uintptr_t PawnFallback2 = 0x0358;
        constexpr uintptr_t CameraManager = 0x03C8;
        constexpr uintptr_t MouseFlags    = 0x0548;
    }

    namespace CameraManager {
        constexpr uintptr_t CameraLocation = 0x19F0;
        constexpr uintptr_t CameraRotation = 0x19FC;
        constexpr uintptr_t CameraFOV      = 0x1A08;
        constexpr uintptr_t DefaultFOV     = 0x0340;
    }

    namespace Human {
        constexpr uintptr_t RootComponent     = 0x0158;
        constexpr uintptr_t CapsuleComponent  = 0x0398;
        constexpr uintptr_t CapsuleFallback   = 0x0330;
        constexpr uintptr_t MovementComponent = 0x0390;
        constexpr uintptr_t MeshComponent     = 0x0388;
        constexpr uintptr_t ActorFlags        = 0x0084;
        constexpr uintptr_t GravityFlags      = 0x042C;
        constexpr uintptr_t SpringArm         = 0x0838;
        constexpr uintptr_t CameraComponent   = 0x0840;
        constexpr uintptr_t Invisibility      = 0x0E2A;
        constexpr uintptr_t BlockFly          = 0x0E2D;
        constexpr uintptr_t PosGrave          = 0x0E30;
    }

    namespace Movement {
        constexpr uintptr_t Velocity                  = 0x0104;
        constexpr uintptr_t GravityScale              = 0x01A4;
        constexpr uintptr_t MovementMode              = 0x01BC;
        constexpr uintptr_t MaxFlySpeed               = 0x01F0;
        constexpr uintptr_t BrakingDecelerationFlying = 0x0218;
        constexpr uintptr_t CheatFlying               = 0x03ED;
    }

    namespace SceneComponent {
        constexpr uintptr_t RelativeLocation   = 0x015C;
        constexpr uintptr_t CollisionEnabled   = 0x02C4;
        constexpr uintptr_t OwnerNoSee         = 0x0252;
        constexpr uintptr_t CapsuleHalfHeight  = 0x0580;
    }

    namespace SpringArm {
        constexpr uintptr_t TargetArmLength    = 0x0238;
        constexpr uintptr_t SocketOffset       = 0x023C;
        constexpr uintptr_t TargetOffset       = 0x0248;
        constexpr uintptr_t CollisionFlags     = 0x025C;
    }

    namespace CameraComponent {
        constexpr uintptr_t FieldOfView        = 0x0238;
    }

    namespace Level {
        constexpr uintptr_t Actors             = 0x00A0;
        constexpr uintptr_t ActorCount         = 0x00A8;
    }

    namespace Neighbor {
        constexpr uintptr_t MeshComponent          = 0x02F0;
        constexpr uintptr_t MovementComponent      = 0x02F8;
        constexpr uintptr_t HeadData               = 0x0830;
        constexpr uintptr_t Scanner                = 0x08A8;
        constexpr uintptr_t ScannerSightRadius     = 0x0238;
        constexpr uintptr_t ScannerPeripheralAngle = 0x0240;
        constexpr uintptr_t BehaviorsData          = 0x0A48;
        constexpr uintptr_t BehaviorsCount         = 0x0A50;
    }

    namespace Item {
        constexpr uintptr_t FlatMesh               = 0x0438;
    }

    namespace UObject {
        constexpr uintptr_t Class                  = 0x0010;
        constexpr uintptr_t Name                   = 0x0018;
    }

    namespace UStruct {
        constexpr uintptr_t SuperStruct            = 0x0030;
        constexpr uintptr_t Children               = 0x0038;
        constexpr uintptr_t Next                   = 0x0028;
    }

}
