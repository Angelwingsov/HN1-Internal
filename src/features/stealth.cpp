#include "stealth.hpp"
#include "../config/settings.hpp"
#include "../core/memory.hpp"
#include "../game/offsets.hpp"
#include "../game/ue_enums.hpp"

namespace Features::Stealth {

    using namespace Core::Memory;
    using namespace Game;

    struct SensorBackup {
        float sightRadius = 0.0f;
        float peripheralAngle = 0.0f;
        UE::FHeadData headData{};
        bool isSaved = false;
    };

    static SensorBackup s_backup;
    static uintptr_t s_cachedNeighbor = 0;
    static bool s_sensorsBlinded = false;

    void UpdateLocalPlayer(uintptr_t localPawn) {
        if (!IsValidPtr(localPawn))
            return;

        const auto& settings = Config::Get().stealth;
        SafeWrite<uint8_t>(localPawn + Offsets::Human::Invisibility, settings.invisible ? 1 : 0);

        if (!settings.invisible && s_sensorsBlinded && IsValidPtr(s_cachedNeighbor)) {
            ProcessNeighbor(s_cachedNeighbor);
        }
    }

    void ProcessNeighbor(uintptr_t neighborActor) {
        if (!IsValidPtr(neighborActor))
            return;

        s_cachedNeighbor = neighborActor;
        const auto& settings = Config::Get().stealth;
        const uintptr_t scanner = SafeRead<uintptr_t>(neighborActor + Offsets::Neighbor::Scanner);

        if (settings.invisible) {
            if (!s_backup.isSaved) {
                if (scanner) {
                    s_backup.sightRadius = SafeRead<float>(scanner + Offsets::Neighbor::ScannerSightRadius);
                    s_backup.peripheralAngle = SafeRead<float>(scanner + Offsets::Neighbor::ScannerPeripheralAngle);
                }
                s_backup.headData = SafeRead<UE::FHeadData>(neighborActor + Offsets::Neighbor::HeadData);
                s_backup.isSaved = true;
            }

            if (scanner) {
                SafeWrite<float>(scanner + Offsets::Neighbor::ScannerSightRadius, 0.0f);
                SafeWrite<float>(scanner + Offsets::Neighbor::ScannerPeripheralAngle, 0.0f);
            }
            const UE::FHeadData zeroHead{};
            SafeWrite<UE::FHeadData>(neighborActor + Offsets::Neighbor::HeadData, zeroHead);
            s_sensorsBlinded = true;

        } else if (s_sensorsBlinded && s_backup.isSaved) {
            if (scanner) {
                SafeWrite<float>(scanner + Offsets::Neighbor::ScannerSightRadius, s_backup.sightRadius);
                SafeWrite<float>(scanner + Offsets::Neighbor::ScannerPeripheralAngle, s_backup.peripheralAngle);
            }
            SafeWrite<UE::FHeadData>(neighborActor + Offsets::Neighbor::HeadData, s_backup.headData);
            s_sensorsBlinded = false;
        }
    }

}
