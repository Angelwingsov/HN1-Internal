#pragma once
#include <windows.h>
#include <cstdint>
#include <vector>
#include <string_view>

namespace Core::Memory {

    constexpr uintptr_t MinUserAddress = 0x10000;
    constexpr uintptr_t MaxUserAddress = 0x7FFFFFFFFFFF;

    inline bool IsValidPtr(uintptr_t addr) {
        return (addr >= MinUserAddress && addr <= MaxUserAddress);
    }

    template<typename T>
    inline T SafeRead(uintptr_t addr, T fallback = T{}) {
        if (!IsValidPtr(addr))
            return fallback;

        __try {
            return *reinterpret_cast<const T*>(addr);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            return fallback;
        }
    }

    template<typename T>
    inline bool SafeWrite(uintptr_t addr, const T& val) {
        if (!IsValidPtr(addr))
            return false;

        __try {
            *reinterpret_cast<T*>(addr) = val;
            return true;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            return false;
        }
    }

    inline uintptr_t GetModuleBase(const wchar_t* moduleName = nullptr) {
        return reinterpret_cast<uintptr_t>(GetModuleHandleW(moduleName));
    }

    inline uintptr_t FindPattern(uintptr_t moduleBase, const char* signature, const char* mask) {
        if (!IsValidPtr(moduleBase))
            return 0;

        const auto dosHeader = reinterpret_cast<const IMAGE_DOS_HEADER*>(moduleBase);
        if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
            return 0;

        const auto ntHeaders = reinterpret_cast<const IMAGE_NT_HEADERS*>(moduleBase + dosHeader->e_lfanew);
        if (ntHeaders->Signature != IMAGE_NT_SIGNATURE)
            return 0;

        const size_t imageSize = ntHeaders->OptionalHeader.SizeOfImage;
        const auto scanBytes = reinterpret_cast<const uint8_t*>(moduleBase);
        const auto patternBytes = reinterpret_cast<const uint8_t*>(signature);
        const size_t patternLength = strlen(mask);

        for (size_t i = 0; i < imageSize - patternLength; ++i) {
            bool found = true;
            for (size_t j = 0; j < patternLength; ++j) {
                if (mask[j] != '?' && scanBytes[i + j] != patternBytes[j]) {
                    found = false;
                    break;
                }
            }
            if (found) {
                return moduleBase + i;
            }
        }
        return 0;
    }

    inline uintptr_t ResolveRelativeAddress(uintptr_t instructionAddr, int offsetToDisplacement, int instructionLength) {
        if (!IsValidPtr(instructionAddr))
            return 0;

        const int32_t ripOffset = SafeRead<int32_t>(instructionAddr + offsetToDisplacement);
        return instructionAddr + instructionLength + ripOffset;
    }

}
