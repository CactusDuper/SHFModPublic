// Credit: https://github.com/Sh0ckFR/Universal-Dear-ImGui-Hook
#include "stdafx.h"
#include "Globals.hpp"

using Microsoft::WRL::ComPtr;

namespace imguihooks {
    // VTable indices derived from the official DirectX interface order.
    // These values are stable across Windows versions and SDKs.
    constexpr size_t kPresentIndex  = 8;            // IDXGISwapChain::Present
    constexpr size_t kResizeBuffersIndex = 13;     // IDXGISwapChain::ResizeBuffers
    constexpr size_t kExecuteCommandListsIndex = 10; // ID3D12CommandQueue::ExecuteCommandLists
    // Dummy objects pour extraire les v-tables
    static ComPtr<IDXGISwapChain3>       pSwapChain = nullptr;
    static ComPtr<ID3D12Device>          pDevice = nullptr;
    static ComPtr<ID3D12CommandQueue>    pCommandQueue = nullptr;
    static HWND                          hDummyWindow = nullptr;
    static const wchar_t* dummyClassName = L"DummyWndClass";

    static LPVOID pPresentTarget = nullptr;
    static LPVOID pPresent1Target = nullptr;
    static LPVOID pResizeBuffersTarget = nullptr;
    static LPVOID pExecuteCommandListsTarget = nullptr;

    static void CleanupDummyObjects() {
        if (hDummyWindow) {
            DestroyWindow(hDummyWindow);
            hDummyWindow = nullptr;
        }

        UnregisterClassW(dummyClassName, GetModuleHandle(nullptr));

        pSwapChain.Reset();
        pDevice.Reset();
        pCommandQueue.Reset();
    }

    // Create hidden Window + device + DX12 swapchain
    static HRESULT CreateDeviceAndSwapChain() {
        // 1) Register dummy window
        WNDCLASSEXW wc = {
            sizeof(WNDCLASSEXW),
            CS_CLASSDC,
            DefWindowProcW,
            0, 0,
            GetModuleHandleW(nullptr),
            nullptr, nullptr, nullptr, nullptr,
            dummyClassName,
            nullptr
        };
        if (!RegisterClassExW(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
            return E_FAIL;
        }

        // 2) Create hidden window
        hDummyWindow = CreateWindowExW(
            0, dummyClassName, L"Dummy",
            WS_OVERLAPPEDWINDOW,
            0, 0, 1, 1,
            nullptr, nullptr, wc.hInstance, nullptr
        );
        if (!hDummyWindow) {
            return E_FAIL;
        }

        // 3) Factory DXGI
        ComPtr<IDXGIFactory4> factory;
        HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
        if (FAILED(hr)) {
            return hr;
        }

        // 4) Device D3D12
        hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&pDevice));
        if (FAILED(hr)) {
            return hr;
        }

        // 5) Command Queue
        D3D12_COMMAND_QUEUE_DESC cqDesc = {};
        cqDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        cqDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        hr = pDevice->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&pCommandQueue));
        if (FAILED(hr)) {
            return hr;
        }

        // 6) SwapChainDesc1
        DXGI_SWAP_CHAIN_DESC1 scDesc = {};
        scDesc.BufferCount = 2;
        scDesc.Width = 1;
        scDesc.Height = 1;
        scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        scDesc.SampleDesc.Count = 1;

        ComPtr<IDXGISwapChain1> swapChain1;
        hr = factory->CreateSwapChainForHwnd(
            pCommandQueue.Get(),
            hDummyWindow,
            &scDesc,
            nullptr, nullptr,
            &swapChain1
        );
        if (FAILED(hr)) {
            return hr;
        }

        // 7) Query IDXGISwapChain3
        hr = swapChain1.As(&pSwapChain);
        if (FAILED(hr)) {
            return hr;
        }

        return S_OK;
    }

    void Init() {
        struct CleanupGuard {
            ~CleanupGuard() { CleanupDummyObjects(); }
        } cleanup;

        if (FAILED(CreateDeviceAndSwapChain())) {
            return;
        }

        // --- Hook Present on SwapChain ---
        auto scVTable = *reinterpret_cast<void***>(pSwapChain.Get());
        pPresentTarget = reinterpret_cast<LPVOID>(scVTable[kPresentIndex]);
        MH_CreateHook(
            pPresentTarget,
            reinterpret_cast<LPVOID>(d3d12hook::HkPresentD3D12),
            reinterpret_cast<LPVOID*>(&d3d12hook::oPresentD3D12)
        );


        // --- Hook ResizeBuffers ---
        pResizeBuffersTarget = reinterpret_cast<LPVOID>(scVTable[kResizeBuffersIndex]);
        MH_CreateHook(
            pResizeBuffersTarget,
            reinterpret_cast<LPVOID>(d3d12hook::HkResizeBuffersD3D12),
            reinterpret_cast<LPVOID*>(&d3d12hook::oResizeBuffersD3D12)
        );

        // --- Hook ExecuteCommandLists ---
        auto cqVTable = *reinterpret_cast<void***>(pCommandQueue.Get());
        pExecuteCommandListsTarget = reinterpret_cast<LPVOID>(cqVTable[kExecuteCommandListsIndex]);
        MH_CreateHook(
            pExecuteCommandListsTarget,
            reinterpret_cast<LPVOID>(d3d12hook::HkExecuteCommandListsD3D12),
            reinterpret_cast<LPVOID*>(&d3d12hook::oExecuteCommandListsD3D12)
        );

        // --- Enable all hooks ---
        MH_EnableHook(MH_ALL_HOOKS);
    }

    void Remove() {
        if (pPresentTarget) {
            MH_DisableHook(pPresentTarget);
            MH_RemoveHook(pPresentTarget);
            pPresentTarget = nullptr;
        }
        if (pPresent1Target) {
            MH_DisableHook(pPresent1Target);
            MH_RemoveHook(pPresent1Target);
            pPresent1Target = nullptr;
        }
        if (pResizeBuffersTarget) {
            MH_DisableHook(pResizeBuffersTarget);
            MH_RemoveHook(pResizeBuffersTarget);
            pResizeBuffersTarget = nullptr;
        }
        if (pExecuteCommandListsTarget) {
            MH_DisableHook(pExecuteCommandListsTarget);
            MH_RemoveHook(pExecuteCommandListsTarget);
            pExecuteCommandListsTarget = nullptr;
        }
    }
}
