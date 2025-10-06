// Credit: https://github.com/Sh0ckFR/Universal-Dear-ImGui-Hook
#include "stdafx.h"
#include "Globals.hpp"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace inputhook {
    static WNDPROC sOriginalWndProc = nullptr;

    // Cache the window handle to allow later removal of the hook
    // (stored in globals::mainWindow for cross-namespace access)

    void Init(HWND hWindow) {

        // Store window globally for later use during release
        GMainWindow = hWindow;

        sOriginalWndProc = (WNDPROC)SetWindowLongPtr(hWindow, GWLP_WNDPROC, (LONG_PTR)WndProc);
    }

    void Remove(HWND hWindow) {
        if (!sOriginalWndProc) {
            return;
        }

        SetWindowLongPtr(hWindow, GWLP_WNDPROC, (LONG_PTR)sOriginalWndProc);

        // Clear cached values to prevent repeated removals
        sOriginalWndProc = nullptr;
        GMainWindow = nullptr;
    }

    LRESULT APIENTRY WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        if (GIsGUIOpen) {
            ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam);
            ImGuiIO& io = ImGui::GetIO();
            if (io.WantCaptureMouse || io.WantCaptureKeyboard) {
                switch (uMsg) {
                case WM_KEYUP:
                case WM_SYSKEYUP:
                case WM_LBUTTONUP:
                case WM_RBUTTONUP:
                case WM_MBUTTONUP:
                case WM_XBUTTONUP:
                    return CallWindowProc(sOriginalWndProc, hwnd, uMsg, wParam, lParam);
                default:
                    return TRUE;
                }
            }
        }

        return CallWindowProc(sOriginalWndProc, hwnd, uMsg, wParam, lParam);
    }
}
