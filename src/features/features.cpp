#include "features.hpp"
#include <vector>
#include "imgui.h"
#include "../config/settings.hpp"
#include "../game/engine.hpp"
#include "noclip.hpp"
#include "stealth.hpp"
#include "camera.hpp"
#include "esp.hpp"

namespace Features {

    using namespace Game;

    void OnTick() {
        const uintptr_t world = Engine::GetWorld();
        const uintptr_t localPawn = Engine::GetLocalPawn();
        const uintptr_t camMgr = Engine::GetCameraManager();

        Engine::SetMouseInput(Config::Get().menu.isOpen);

        Core::Math::Vector3 camLoc;
        Core::Math::Rotator camRot;
        float camFov = 90.0f;

        if (Engine::GetCameraView(camLoc, camRot, camFov)) {
            Noclip::Update(localPawn, world, camLoc, camRot);
        }

        Stealth::UpdateLocalPlayer(localPawn);
        Camera::Update(localPawn, camMgr);
    }

    void OnRender() {
        const auto& config = Config::Get();
        ImDrawList* drawList = ImGui::GetBackgroundDrawList();

        float hudY = 16.0f;

        if (config.menu.showWatermark) {
            drawList->AddText(ImVec2(17.0f, hudY + 1.0f), IM_COL32(0, 0, 0, 220), "HN1 Cheat");
            drawList->AddText(ImVec2(16.0f, hudY), IM_COL32(255, 255, 255, 220), "HN1 Cheat");
            hudY += 18.0f;
        }

        if (config.movement.noclip) {
            drawList->AddText(ImVec2(17.0f, hudY + 1.0f), IM_COL32(0, 0, 0, 220), "Noclip: Active (F1)");
            drawList->AddText(ImVec2(16.0f, hudY), IM_COL32(255, 75, 75, 255), "Noclip: Active (F1)");
            hudY += 18.0f;
        }

        if (config.stealth.invisible) {
            drawList->AddText(ImVec2(17.0f, hudY + 1.0f), IM_COL32(0, 0, 0, 220), "Invisibility: Active (F2)");
            drawList->AddText(ImVec2(16.0f, hudY), IM_COL32(60, 220, 255, 255), "Invisibility: Active (F2)");
            hudY += 18.0f;
        }

        if (config.camera.thirdPerson) {
            drawList->AddText(ImVec2(17.0f, hudY + 1.0f), IM_COL32(0, 0, 0, 220), "Third Person: Active (F3)");
            drawList->AddText(ImVec2(16.0f, hudY), IM_COL32(100, 255, 120, 255), "Third Person: Active (F3)");
            hudY += 18.0f;
        }

        Core::Math::Vector3 camLoc;
        Core::Math::Rotator camRot;
        float camFov = 90.0f;

        if (!Engine::GetCameraView(camLoc, camRot, camFov))
            return;

        const ImGuiIO& io = ImGui::GetIO();
        const float screenW = io.DisplaySize.x;
        const float screenH = io.DisplaySize.y;

        std::vector<uintptr_t> actors;
        if (!Engine::GetLevelActors(actors))
            return;

        const uintptr_t localPawn = Engine::GetLocalPawn();
        ESP::Render(camLoc, camRot, camFov, screenW, screenH, actors, localPawn);
    }

}
