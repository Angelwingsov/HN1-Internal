#pragma once
#include <windows.h>
#include <string>
#include <cstdint>
#include "ue_enums.hpp"

namespace Game::UE {

    struct FStringInternal {
        wchar_t* Data = nullptr;
        int32_t Count = 0;
        int32_t Max   = 0;

        FStringInternal() = default;
        FStringInternal(wchar_t* data, int32_t count, int32_t max)
            : Data(data), Count(count), Max(max) {}
    };

    struct FNameInternal {
        int32_t ComparisonIndex = 0;
        int32_t Number          = 0;
    };

    typedef void(*AppendString_t)(const FNameInternal* Name, FStringInternal& OutString);

    std::string GetNameFromFName(const FNameInternal& fname);
    std::string GetObjectName(uintptr_t uobject);
    std::string GetClassName(uintptr_t uobject);

    void SafeProcessEvent(uintptr_t obj, uintptr_t func, void* params);
    uintptr_t FindFunctionFast(uintptr_t uclass, const std::string& funcName);

}
