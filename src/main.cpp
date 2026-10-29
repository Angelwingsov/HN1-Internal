#include <windows.h>
#include <iostream>
#include "core/hooks.hpp"

#ifdef _DEBUG
static void SetupDebugConsole() {
    AllocConsole();
    FILE* stream;
    freopen_s(&stream, "CONOUT$", "w", stdout);
    freopen_s(&stream, "CONOUT$", "w", stderr);
    std::cout << "========================================" << std::endl;
    std::cout << "  HN1 Cheat - Debug Console             " << std::endl;
    std::cout << "========================================" << std::endl;
}
#endif

static DWORD WINAPI MainThread(LPVOID lpParam) {
    const auto hModule = static_cast<HMODULE>(lpParam);

#ifdef _DEBUG
    SetupDebugConsole();
    std::cout << "[HN1] Initializing Direct3D 11 hooks..." << std::endl;
#endif

    Sleep(1500);

    if (Core::Hooks::Initialize()) {
#ifdef _DEBUG
        std::cout << "[HN1] Hooks initialized successfully." << std::endl;
#endif
    } else {
#ifdef _DEBUG
        std::cout << "[HN1] Failed to initialize hooks!" << std::endl;
#endif
    }

    while (!Core::Hooks::IsUnloadRequested()) {
        Sleep(100);
    }

    Sleep(200);

#ifdef _DEBUG
    FreeConsole();
#endif

    FreeLibraryAndExitThread(hModule, 0);
    return 0;
}

BOOL WINAPI DllMain(HMODULE hMod, DWORD dwReason, LPVOID lpReserved) {
    if (dwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hMod);
        CreateThread(nullptr, 0, MainThread, hMod, 0, nullptr);
    } else if (dwReason == DLL_PROCESS_DETACH && lpReserved == nullptr) {
        Core::Hooks::Shutdown();
    }
    return TRUE;
}
