#include "menu.hpp"
#include "imgui.h"
#include "../config/settings.hpp"
#include "../core/hooks.hpp"

namespace GUI::Menu {

    void Render() {
        auto& config = Config::Get();
        if (!config.menu.isOpen)
            return;

        ImGui::SetNextWindowSize(ImVec2(360.0f, 440.0f), ImGuiCond_FirstUseEver);

        if (ImGui::Begin("HN1 Cheat", &config.menu.isOpen)) {
            if (ImGui::CollapsingHeader("Visuals", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::Checkbox("Neighbor ESP", &config.visuals.espNeighbor);
                if (config.visuals.espNeighbor) {
                    ImGui::Indent(16.0f);
                    ImGui::Checkbox("Show Phase / State", &config.visuals.espPhase);
                    ImGui::Unindent(16.0f);
                }

                ImGui::Checkbox("Items ESP", &config.visuals.espItems);
                if (config.visuals.espItems) {
                    ImGui::Indent(16.0f);
                    ImGui::Checkbox("Only Key Items", &config.visuals.onlyKeyItems);
                    ImGui::Unindent(16.0f);
                }

                ImGui::Checkbox("Show Distance", &config.visuals.espDistance);
                ImGui::SliderFloat("Max Distance", &config.visuals.maxDistance, 10.0f, 150.0f, "%.0fm");
            }

            if (ImGui::CollapsingHeader("Movement", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::Checkbox("Noclip (F1)", &config.movement.noclip);
                ImGui::SliderFloat("Noclip Speed", &config.movement.noclipSpeed, 10.0f, 120.0f, "%.1f");
            }

            if (ImGui::CollapsingHeader("Stealth", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::Checkbox("Invisibility (F2)", &config.stealth.invisible);
            }

            if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::Checkbox("Third Person (F3)", &config.camera.thirdPerson);
                if (config.camera.thirdPerson) {
                    ImGui::Indent(16.0f);
                    ImGui::SliderFloat("Distance", &config.camera.thirdPersonDist, 50.0f, 600.0f, "%.0f");
                    ImGui::SliderFloat("Height", &config.camera.thirdPersonOffsetZ, -60.0f, 100.0f, "%.0f");
                    ImGui::SliderFloat("Side Offset", &config.camera.thirdPersonOffsetY, -100.0f, 100.0f, "%.0f");
                    if (ImGui::Button("Reset Camera")) {
                        config.camera.thirdPersonDist = 250.0f;
                        config.camera.thirdPersonOffsetZ = 20.0f;
                        config.camera.thirdPersonOffsetY = 35.0f;
                    }
                    ImGui::Unindent(16.0f);
                }

                ImGui::Checkbox("Custom FOV", &config.camera.customFov);
                if (config.camera.customFov) {
                    ImGui::Indent(16.0f);
                    ImGui::SliderFloat("FOV", &config.camera.customFovValue, 60.0f, 140.0f, "%.0f");
                    if (ImGui::Button("Reset FOV")) {
                        config.camera.customFovValue = 90.0f;
                    }
                    ImGui::Unindent(16.0f);
                }
            }

            if (ImGui::CollapsingHeader("Misc")) {
                ImGui::Checkbox("Show Watermark", &config.menu.showWatermark);
                ImGui::Separator();
                if (ImGui::Button("Unload Cheat", ImVec2(-1.0f, 24.0f))) {
                    Core::Hooks::RequestUnload();
                }
            }
        }
        ImGui::End();
    }

}
