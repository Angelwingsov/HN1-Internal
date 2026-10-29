#pragma once
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>

namespace Core::Hooks {

    bool Initialize();
    void Shutdown();

    void RequestUnload();
    bool IsUnloadRequested();

}
