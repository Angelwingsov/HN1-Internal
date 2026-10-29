#include "hooks.hpp"
#include <iostream>
#include <atomic>
#include "kiero.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "../config/settings.hpp"
#include "../gui/menu.hpp"
#include "../features/features.hpp"
#include "../game/engine.hpp"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Core::Hooks {

    typedef HRESULT(__stdcall* Present_t)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
    typedef LRESULT(CALLBACK* WNDPROC_t)(HWND, UINT, WPARAM, LPARAM);

    static Present_t s_oPresent = nullptr;
    static WNDPROC_t s_oWndProc = nullptr;
    static HWND s_hWindow = nullptr;

    static ID3D11Device* s_pDevice = nullptr;
    static ID3D11DeviceContext* s_pContext = nullptr;
    static ID3D11RenderTargetView* s_pRenderTarget = nullptr;

    static std::atomic<bool> s_bInitialized{ false };
    static std::atomic<bool> s_bUnloadRequested{ false };

    void RequestUnload() {
        s_bUnloadRequested.store(true);
    }

    bool IsUnloadRequested() {
        return s_bUnloadRequested.load();
    }

    static void CreateRenderTarget(IDXGISwapChain* pSwapChain) {
        ID3D11Texture2D* pBackBuffer = nullptr;
        if (SUCCEEDED(pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer)))) {
            s_pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &s_pRenderTarget);
            pBackBuffer->Release();
        }
    }

    static void ReleaseRenderTarget() {
        if (s_pRenderTarget) {
            s_pRenderTarget->Release();
            s_pRenderTarget = nullptr;
        }
    }

    static HWND FindProcessWindow() {
        const DWORD currentPid = GetCurrentProcessId();
        HWND result = nullptr;

        HWND hwnd = nullptr;
        while ((hwnd = FindWindowExA(nullptr, hwnd, nullptr, nullptr)) != nullptr) {
            DWORD windowPid = 0;
            GetWindowThreadProcessId(hwnd, &windowPid);
            if (windowPid == currentPid && IsWindowVisible(hwnd)) {
                result = hwnd;
                break;
            }
        }
        return result;
    }

    static LRESULT CALLBACK hkWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        if (uMsg == WM_KEYDOWN) {
            auto& config = Config::Get();

            if (wParam == VK_INSERT) {
                config.menu.isOpen = !config.menu.isOpen;
                return 0;
            }
            if (wParam == VK_F1) {
                config.movement.noclip = !config.movement.noclip;
                return 0;
            }
            if (wParam == VK_F2) {
                config.stealth.invisible = !config.stealth.invisible;
                return 0;
            }
            if (wParam == VK_F3) {
                config.camera.thirdPerson = !config.camera.thirdPerson;
                return 0;
            }
        }

        if (Config::Get().menu.isOpen) {
            if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
                return TRUE;
        }

        return CallWindowProc(s_oWndProc, hWnd, uMsg, wParam, lParam);
    }

    static HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) {
        if (!s_bInitialized.load()) {
            DXGI_SWAP_CHAIN_DESC desc;
            pSwapChain->GetDesc(&desc);

            s_hWindow = desc.OutputWindow;
            if (!s_hWindow) {
                s_hWindow = FindProcessWindow();
                if (!s_hWindow)
                    s_hWindow = GetForegroundWindow();
            }

            if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), reinterpret_cast<void**>(&s_pDevice)))) {
                s_pDevice->GetImmediateContext(&s_pContext);
                CreateRenderTarget(pSwapChain);

                s_oWndProc = reinterpret_cast<WNDPROC_t>(
                    SetWindowLongPtr(s_hWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(hkWndProc))
                );

                ImGui::CreateContext();
                ImGuiIO& io = ImGui::GetIO();
                io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

                ImGui::StyleColorsClassic();

                ImGui_ImplWin32_Init(s_hWindow);
                ImGui_ImplDX11_Init(s_pDevice, s_pContext);

                s_bInitialized.store(true);
            } else {
                return s_oPresent(pSwapChain, SyncInterval, Flags);
            }
        }

        if (s_bUnloadRequested.load()) {
            Shutdown();
            return s_oPresent(pSwapChain, SyncInterval, Flags);
        }

        if (!s_pRenderTarget) {
            CreateRenderTarget(pSwapChain);
        }

        Features::OnTick();

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGui::GetIO().MouseDrawCursor = Config::Get().menu.isOpen;

        Features::OnRender();
        GUI::Menu::Render();

        ImGui::Render();
        if (s_pRenderTarget && s_pContext) {
            s_pContext->OMSetRenderTargets(1, &s_pRenderTarget, nullptr);
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        }

        return s_oPresent(pSwapChain, SyncInterval, Flags);
    }

    bool Initialize() {
        constexpr int maxAttempts = 30;
        int attempts = 0;

        while (attempts < maxAttempts) {
            const kiero::Status::Enum status = kiero::init(kiero::RenderType::D3D11);
            if (status == kiero::Status::Success) {
                if (kiero::bind(8, reinterpret_cast<void**>(&s_oPresent), reinterpret_cast<void*>(hkPresent)) == kiero::Status::Success) {
                    return true;
                }
            }
            Sleep(500);
            attempts++;
        }

        return false;
    }

    void Shutdown() {
        if (!s_bInitialized.load())
            return;

        auto& config = Config::Get();
        config.movement.noclip = false;
        config.stealth.invisible = false;
        config.camera.thirdPerson = false;
        config.camera.customFov = false;
        config.menu.isOpen = false;

        Features::OnTick();

        if (s_hWindow && s_oWndProc) {
            SetWindowLongPtr(s_hWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(s_oWndProc));
            s_oWndProc = nullptr;
        }

        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        ReleaseRenderTarget();

        if (s_pContext) {
            s_pContext->Release();
            s_pContext = nullptr;
        }

        if (s_pDevice) {
            s_pDevice->Release();
            s_pDevice = nullptr;
        }

        kiero::shutdown();
        s_bInitialized.store(false);
    }

}
