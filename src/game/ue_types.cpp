#include "ue_types.hpp"
#include "offsets.hpp"
#include "../core/memory.hpp"
#include <unordered_map>

namespace Game::UE {

    using namespace Core::Memory;

    static AppendString_t s_pfnAppendString = nullptr;
    static std::unordered_map<int32_t, std::string> s_nameCache;

    static bool TryCallAppendString(AppendString_t fn, const FNameInternal* pName, FStringInternal& outStr) {
        __try {
            fn(pName, outStr);
            return true;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            return false;
        }
    }

    std::string GetNameFromFName(const FNameInternal& fname) {
        if (fname.ComparisonIndex <= 0)
            return "";

        const auto it = s_nameCache.find(fname.ComparisonIndex);
        if (it != s_nameCache.end())
            return it->second;

        if (!s_pfnAppendString) {
            const uintptr_t base = GetModuleBase();
            if (!base)
                return "";
            s_pfnAppendString = reinterpret_cast<AppendString_t>(base + Offsets::Engine::AppendString);
        }

        if (!s_pfnAppendString)
            return "";

        constexpr int MaxBufferLength = 256;
        wchar_t buffer[MaxBufferLength] = { 0 };
        FStringInternal tempStr(buffer, 0, MaxBufferLength);

        if (TryCallAppendString(s_pfnAppendString, &fname, tempStr)) {
            if (tempStr.Data && tempStr.Count > 0) {
                char mbBuf[MaxBufferLength] = { 0 };
                WideCharToMultiByte(CP_UTF8, 0, tempStr.Data, tempStr.Count, mbBuf, sizeof(mbBuf), nullptr, nullptr);
                std::string result(mbBuf);
                const size_t pos = result.rfind('/');
                if (pos != std::string::npos)
                    result = result.substr(pos + 1);

                s_nameCache[fname.ComparisonIndex] = result;
                return result;
            }
        }

        return "";
    }

    std::string GetObjectName(uintptr_t uobject) {
        if (!IsValidPtr(uobject))
            return "";

        const auto fn = SafeRead<FNameInternal>(uobject + Offsets::UObject::Name);
        return GetNameFromFName(fn);
    }

    std::string GetClassName(uintptr_t uobject) {
        if (!IsValidPtr(uobject))
            return "";

        const uintptr_t uclass = SafeRead<uintptr_t>(uobject + Offsets::UObject::Class);
        if (!uclass)
            return "";

        return GetObjectName(uclass);
    }

    void SafeProcessEvent(uintptr_t obj, uintptr_t func, void* params) {
        if (!IsValidPtr(obj) || !IsValidPtr(func))
            return;

        __try {
            const uintptr_t vtable = *reinterpret_cast<uintptr_t*>(obj);
            if (!vtable)
                return;

            typedef void(*ProcessEventFn)(uintptr_t, uintptr_t, void*);
            const ProcessEventFn pe = *reinterpret_cast<ProcessEventFn*>(
                vtable + Offsets::Engine::ProcessEventVTableIndex * sizeof(void*)
            );

            if (pe)
                pe(obj, func, params);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
        }
    }

    uintptr_t FindFunctionFast(uintptr_t uclass, const std::string& funcName) {
        if (!IsValidPtr(uclass))
            return 0;

        for (uintptr_t c = uclass; c; c = SafeRead<uintptr_t>(c + Offsets::UStruct::SuperStruct)) {
            for (uintptr_t field = SafeRead<uintptr_t>(c + Offsets::UStruct::Children);
                 field;
                 field = SafeRead<uintptr_t>(field + Offsets::UStruct::Next)) {
                
                if (GetObjectName(field) == funcName)
                    return field;
            }
        }
        return 0;
    }

}
