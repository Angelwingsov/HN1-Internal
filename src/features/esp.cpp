#include "esp.hpp"
#include "stealth.hpp"
#include <algorithm>
#include <string_view>
#include <cstdio>
#include "imgui.h"
#include "../config/settings.hpp"
#include "../core/memory.hpp"
#include "../game/offsets.hpp"
#include "../game/ue_types.hpp"
#include "../game/ue_enums.hpp"

namespace Features::ESP {

    using namespace Core::Math;
    using namespace Core::Memory;
    using namespace Game;

    struct ItemRule {
        const char* pattern;
        const char* label;
    };

    static constexpr ItemRule s_itemRules[] = {
        { "crowbar",     "Crowbar" },
        { "wrench",      "Wrench" },
        { "key_card",    "Keycard" },
        { "keycard",     "Keycard" },
        { "keycar",      "Car Key" },
        { "lockpick",    "Lockpick" },
        { "key",         "Key" },
        { "battery",     "Battery" },
        { "magnet",      "Magnet" },
        { "trap",        "Trap" },
        { "flashlight",  "Flashlight" },
        { "glue",        "Glue" },
        { "camera",      "Camera" },
        { "binocular",   "Binoculars" },
        { "umbrella",    "Umbrella" },
        { "gun",         "Rifle" },
        { "rifle",       "Rifle" },
        { "apple",       "Apple" },
        { "kettle",      "Kettle" },
        { "bucket",      "Bucket" },
        { "generator",   "Generator" }
    };

    static constexpr const char* s_questBlacklist[] = {
        "spawn", "navlink", "proxy", "area", "animpoint"
    };

    static constexpr const char* s_questWhitelist[] = {
        "key", "crowbar", "wrench", "lockpick", "card", "battery", "magnet", "trap"
    };

    struct BehaviorRule {
        const char* keyword;
        const char* label;
        ImU32 color;
    };

    static const BehaviorRule s_behaviorRules[] = {
        { "hunt",        "Hunting",               IM_COL32(255, 60, 60, 255) },
        { "attack",      "Attacking",             IM_COL32(255, 30, 30, 255) },
        { "door",        "Searching (Door)",      IM_COL32(255, 140, 0, 255) },
        { "bed",         "Searching (Bed)",       IM_COL32(255, 140, 0, 255) },
        { "cupboard",    "Searching (Closet)",    IM_COL32(255, 140, 0, 255) },
        { "hiding",      "Searching (Hiding)",    IM_COL32(255, 140, 0, 255) },
        { "check",       "Searching",             IM_COL32(255, 140, 0, 255) },
        { "lookaround",  "Searching",             IM_COL32(255, 140, 0, 255) },
        { "intercept",   "Searching",             IM_COL32(255, 140, 0, 255) },
        { "sleep",       "Sleeping",              IM_COL32(100, 180, 255, 255) },
        { "stun",        "Stunned",               IM_COL32(180, 100, 255, 255) },
        { "fall",        "Stunned",               IM_COL32(180, 100, 255, 255) },
        { "trap",        "Setting Trap",          IM_COL32(255, 255, 50, 255) },
        { "setbucket",   "Setting Trap",          IM_COL32(255, 255, 50, 255) },
        { "setcamera",   "Setting Trap",          IM_COL32(255, 255, 50, 255) },
        { "drink",       "Drinking Milk",         IM_COL32(120, 255, 120, 255) },
        { "tv",          "Watching TV",           IM_COL32(120, 255, 120, 255) },
        { "toilet",      "In Toilet",             IM_COL32(120, 255, 120, 255) },
        { "wash",        "Washing",               IM_COL32(120, 255, 120, 255) },
        { "repair",      "Repairing",             IM_COL32(120, 255, 120, 255) },
        { "housework",   "Routine",               IM_COL32(120, 255, 120, 255) },
        { "radio",       "Routine",               IM_COL32(120, 255, 120, 255) },
        { "move",        "Patrolling",            IM_COL32(255, 230, 0, 255) },
        { "elevator",    "Moving (Elevator)",     IM_COL32(255, 230, 0, 255) },
        { "ladder",      "Moving (Ladder)",       IM_COL32(255, 230, 0, 255) },
        { "idle",        "Idle",                  IM_COL32(200, 200, 200, 255) },
        { "wait",        "Idle",                  IM_COL32(200, 200, 200, 255) }
    };

    static std::string ToLower(std::string_view str) {
        std::string result(str);
        std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return result;
    }

    std::string FormatItemName(const std::string& raw) {
        const std::string lower = ToLower(raw);

        for (const auto& rule : s_itemRules) {
            if (lower.find(rule.pattern) != std::string::npos) {
                return rule.label;
            }
        }

        std::string clean = raw;
        if (clean.rfind("BP_", 0) == 0)
            clean = clean.substr(3);

        const size_t lastUnderscore = clean.rfind('_');
        if (lastUnderscore != std::string::npos && lastUnderscore + 1 < clean.size()) {
            bool allDigits = true;
            for (size_t i = lastUnderscore + 1; i < clean.size(); ++i) {
                if (!std::isdigit(static_cast<unsigned char>(clean[i]))) {
                    allDigits = false;
                    break;
                }
            }
            if (allDigits)
                clean = clean.substr(0, lastUnderscore);
        }

        if (clean.size() > 2 && clean.substr(clean.size() - 2) == "_C")
            clean = clean.substr(0, clean.size() - 2);

        for (char& ch : clean) {
            if (ch == '_') ch = ' ';
        }

        return clean;
    }

    bool IsKeyQuestItem(const std::string& name) {
        const std::string lower = ToLower(name);

        for (const auto* blacklisted : s_questBlacklist) {
            if (lower.find(blacklisted) != std::string::npos)
                return false;
        }

        for (const auto* whitelisted : s_questWhitelist) {
            if (lower.find(whitelisted) != std::string::npos)
                return true;
        }

        return false;
    }

    PhaseInfo GetNeighborPhase(uintptr_t sosedActor) {
        static uintptr_t s_fnGetState = 0;
        static uintptr_t s_fnGetBehavior = 0;
        static uintptr_t s_fnIsVisibleHuman = 0;
        static uintptr_t s_fnIsFreeze = 0;
        static uintptr_t s_sosedClass = 0;

        const uintptr_t sosedClass = SafeRead<uintptr_t>(sosedActor + Offsets::UObject::Class);
        if (sosedClass && sosedClass != s_sosedClass) {
            s_sosedClass = sosedClass;
            s_fnGetState = UE::FindFunctionFast(sosedClass, "GetState");
            s_fnGetBehavior = UE::FindFunctionFast(sosedClass, "GetBehavior");
            s_fnIsVisibleHuman = UE::FindFunctionFast(sosedClass, "IsVisibleHuman");
            s_fnIsFreeze = UE::FindFunctionFast(sosedClass, "IsFreeze");
        }

        if (s_fnIsFreeze) {
            bool bFreeze = false;
            UE::SafeProcessEvent(sosedActor, s_fnIsFreeze, &bFreeze);
            if (bFreeze) {
                return { "Stunned", IM_COL32(180, 100, 255, 255) };
            }
        }

        uint8_t nState = 0xFF;
        if (s_fnGetState) {
            UE::SafeProcessEvent(sosedActor, s_fnGetState, &nState);
        }

        bool bSeesPlayer = false;
        if (s_fnIsVisibleHuman) {
            UE::SafeProcessEvent(sosedActor, s_fnIsVisibleHuman, &bSeesPlayer);
        }

        if (nState == static_cast<uint8_t>(UE::ENeighborState::NS_HUNT)) {
            return { bSeesPlayer ? "Hunting (Sees You)" : "Hunting", IM_COL32(255, 50, 50, 255) };
        }
        if (nState == static_cast<uint8_t>(UE::ENeighborState::NS_ATTACK)) {
            return { "Attacking", IM_COL32(255, 20, 20, 255) };
        }

        uintptr_t activeBehavior = 0;
        if (s_fnGetBehavior) {
            UE::SafeProcessEvent(sosedActor, s_fnGetBehavior, &activeBehavior);
        }
        if (!activeBehavior) {
            const uintptr_t behaviorsData = SafeRead<uintptr_t>(sosedActor + Offsets::Neighbor::BehaviorsData);
            const int32_t behaviorsCount = SafeRead<int32_t>(sosedActor + Offsets::Neighbor::BehaviorsCount);
            if (behaviorsData && behaviorsCount > 0 && behaviorsCount < 16) {
                activeBehavior = SafeRead<uintptr_t>(behaviorsData + ((behaviorsCount - 1) * sizeof(void*)));
            }
        }

        if (activeBehavior) {
            std::string behClassName = UE::GetClassName(activeBehavior);
            if (behClassName.empty())
                behClassName = UE::GetObjectName(activeBehavior);

            const std::string nameLower = ToLower(behClassName);

            for (const auto& rule : s_behaviorRules) {
                if (nameLower.find(rule.keyword) != std::string::npos) {
                    return { rule.label, rule.color };
                }
            }

            std::string clean = behClassName;
            if (clean.rfind("Beh", 0) == 0)
                clean = clean.substr(3);
            if (!clean.empty() && clean != "avior" && clean != "ehavior") {
                return { clean, IM_COL32(255, 220, 50, 255) };
            }
        }

        if (bSeesPlayer) {
            return { "Suspicious (Sees You)", IM_COL32(255, 120, 0, 255) };
        }

        return { "Idle", IM_COL32(200, 200, 200, 255) };
    }

    void Render(
        const Vector3& camLoc,
        const Rotator& camRot,
        float camFov,
        float screenW,
        float screenH,
        const std::vector<uintptr_t>& actors,
        uintptr_t localPawn
    ) {
        const auto& settings = Config::Get().visuals;
        ImDrawList* drawList = ImGui::GetBackgroundDrawList();

        for (uintptr_t actor : actors) {
            if (!IsValidPtr(actor) || actor == localPawn)
                continue;

            const uintptr_t rootComp = SafeRead<uintptr_t>(actor + Offsets::Human::RootComponent);
            if (!rootComp)
                continue;

            const Vector3 actorLoc = SafeRead<Vector3>(rootComp + Offsets::SceneComponent::RelativeLocation);
            const float distMeters = camLoc.DistTo(actorLoc) / 100.0f;

            if (distMeters <= 0.2f || distMeters > settings.maxDistance)
                continue;

            const std::string rawName = UE::GetObjectName(actor);
            const std::string className = UE::GetClassName(actor);

            bool isNeighbor = (className == "BP_Sosed_C" || className == "Sosed" ||
                               rawName.rfind("BP_Sosed_C", 0) == 0 || rawName.rfind("Sosed_", 0) == 0);

            if (isNeighbor) {
                const uintptr_t meshComp = SafeRead<uintptr_t>(actor + Offsets::Neighbor::MeshComponent);
                const uintptr_t moveComp = SafeRead<uintptr_t>(actor + Offsets::Neighbor::MovementComponent);
                if (!meshComp || !moveComp) {
                    isNeighbor = false;
                }
            }

            if (isNeighbor) {
                Stealth::ProcessNeighbor(actor);

                if (!settings.espNeighbor)
                    continue;

                float halfHeight = Offsets::Constants::DefaultCapsuleHalfHeight;
                uintptr_t capsule = SafeRead<uintptr_t>(actor + Offsets::Human::CapsuleFallback);
                if (!capsule) capsule = SafeRead<uintptr_t>(actor + Offsets::Human::CapsuleComponent);
                if (capsule) {
                    const float h = SafeRead<float>(capsule + Offsets::SceneComponent::CapsuleHalfHeight);
                    if (h > 40.0f && h < 150.0f)
                        halfHeight = h;
                }

                Vector3 feetLoc = actorLoc;
                feetLoc.Z -= halfHeight;

                Vector3 headLoc = actorLoc;
                headLoc.Z += (halfHeight + 15.0f);

                Vector2 screenFeet, screenHead;
                if (WorldToScreen(feetLoc, camLoc, camRot, camFov, screenW, screenH, screenFeet) &&
                    WorldToScreen(headLoc, camLoc, camRot, camFov, screenW, screenH, screenHead)) {

                    const float boxH = std::fabs(screenFeet.Y - screenHead.Y);
                    const float boxW = boxH * Offsets::Constants::BoxWidthRatio;

                    const ImVec2 pMin(screenFeet.X - boxW * 0.5f, screenHead.Y);
                    const ImVec2 pMax(screenFeet.X + boxW * 0.5f, screenFeet.Y);

                    const PhaseInfo phase = GetNeighborPhase(actor);

                    drawList->AddRect(pMin, pMax, phase.color, 0.0f, 0, 1.5f);
                    drawList->AddRect(ImVec2(pMin.x - 1, pMin.y - 1), ImVec2(pMax.x + 1, pMax.y + 1), IM_COL32(0, 0, 0, 200), 0.0f, 0, 1.0f);

                    char txtTitle[64];
                    std::snprintf(txtTitle, sizeof(txtTitle), "Neighbor (%.0fm)", distMeters);
                    const ImVec2 txtSz = ImGui::CalcTextSize(txtTitle);
                    drawList->AddText(ImVec2(screenFeet.X - txtSz.x * 0.5f + 1, pMin.y - 30.0f + 1), IM_COL32(0, 0, 0, 230), txtTitle);
                    drawList->AddText(ImVec2(screenFeet.X - txtSz.x * 0.5f, pMin.y - 30.0f), IM_COL32(255, 255, 255, 255), txtTitle);

                    if (settings.espPhase) {
                        const ImVec2 phaseSz = ImGui::CalcTextSize(phase.name.c_str());
                        drawList->AddText(ImVec2(screenFeet.X - phaseSz.x * 0.5f + 1, pMin.y - 16.0f + 1), IM_COL32(0, 0, 0, 230), phase.name.c_str());
                        drawList->AddText(ImVec2(screenFeet.X - phaseSz.x * 0.5f, pMin.y - 16.0f), phase.color, phase.name.c_str());
                    }
                }
                continue;
            }

            const std::string combinedName = rawName + " " + className;
            const bool isKeyItem = IsKeyQuestItem(combinedName);

            if (settings.onlyKeyItems && !isKeyItem)
                continue;

            if (!isKeyItem) {
                const uintptr_t flatMesh = SafeRead<uintptr_t>(actor + Offsets::Item::FlatMesh);
                if (!flatMesh)
                    continue;
            }

            if (settings.espItems) {
                Vector2 screenPos;
                if (WorldToScreen(actorLoc, camLoc, camRot, camFov, screenW, screenH, screenPos)) {
                    const std::string displayName = FormatItemName(rawName.empty() ? className : rawName);
                    const ImU32 col = isKeyItem ? IM_COL32(255, 220, 50, 255) : IM_COL32(200, 200, 200, 255);

                    char txt[128];
                    if (settings.espDistance) {
                        std::snprintf(txt, sizeof(txt), "%s (%.0fm)", displayName.c_str(), distMeters);
                    } else {
                        std::snprintf(txt, sizeof(txt), "%s", displayName.c_str());
                    }

                    const ImVec2 txtSz = ImGui::CalcTextSize(txt);
                    drawList->AddText(ImVec2(screenPos.X - txtSz.x * 0.5f + 1, screenPos.Y - txtSz.y * 0.5f + 1), IM_COL32(0, 0, 0, 230), txt);
                    drawList->AddText(ImVec2(screenPos.X - txtSz.x * 0.5f, screenPos.Y - txtSz.y * 0.5f), col, txt);
                }
            }
        }
    }

}
