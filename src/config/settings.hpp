#pragma once

namespace Config {

    struct Visuals {
        bool espNeighbor = true;
        bool espPhase = true;
        bool espItems = true;
        bool onlyKeyItems = true;
        bool espDistance = true;
        float maxDistance = 60.0f;
    };

    struct Movement {
        bool noclip = false;
        float noclipSpeed = 40.0f;
    };

    struct Stealth {
        bool invisible = false;
    };

    struct Camera {
        bool thirdPerson = false;
        float thirdPersonDist = 250.0f;
        float thirdPersonOffsetZ = 20.0f;
        float thirdPersonOffsetY = 35.0f;

        bool customFov = false;
        float customFovValue = 100.0f;
    };

    struct Menu {
        bool isOpen = true;
        bool showWatermark = true;
    };

    struct State {
        Visuals visuals;
        Movement movement;
        Stealth stealth;
        Camera camera;
        Menu menu;
    };

    inline State& Get() {
        static State instance;
        return instance;
    }

}
