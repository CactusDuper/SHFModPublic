// Credit: https://github.com/Sh0ckFR/Universal-Dear-ImGui-Hook
#include "stdafx.h"
#include "Globals.hpp"

namespace imguihooks { void Remove(); }

namespace d3d12hook {
    PresentD3D12            oPresentD3D12 = nullptr;
    ExecuteCommandListsFn   oExecuteCommandListsD3D12 = nullptr;
    ResizeBuffersFn         oResizeBuffersD3D12 = nullptr;

    static ID3D12Device*              gDevice = nullptr;
    static ID3D12CommandQueue*        gCommandQueue = nullptr;
    static ID3D12DescriptorHeap*      gHeapRTV = nullptr;
    static ID3D12DescriptorHeap*      gHeapSRV = nullptr;
    static ID3D12GraphicsCommandList* gCommandList = nullptr;
    static ID3D12Fence*               gOverlayFence = nullptr;
    static HANDLE                     gFenceEvent = nullptr;
    static UINT64                     gOverlayFenceValue = 0;
    static uint64_t                   gBufferCount = 0;

    struct FrameContext {
        ID3D12CommandAllocator* allocator;
        ID3D12Resource* renderTarget;
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
    };
    static FrameContext*          gFrameContexts = nullptr;
    static bool                   gInitialized = false;
    static bool                   gShutdown = false;
    static bool                   gAfterFirstPresent = false;

    void release();

    long __fastcall HkPresentD3D12(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags) {
        if (GetAsyncKeyState(GOpenGUIKey) & 1) {
            GIsGUIOpen = !GIsGUIOpen;
        }

        gAfterFirstPresent = true;
        if (!gCommandQueue) {
            if (!gDevice) {
                pSwapChain->GetDevice(__uuidof(ID3D12Device), (void**)&gDevice);
            }
            return oPresentD3D12(pSwapChain, SyncInterval, Flags);
        }

        if (!gInitialized) {
            if (FAILED(pSwapChain->GetDevice(__uuidof(ID3D12Device), (void**)&gDevice))) {
                return oPresentD3D12(pSwapChain, SyncInterval, Flags);
            }

            // Swap Chain description
            DXGI_SWAP_CHAIN_DESC desc = {};
            pSwapChain->GetDesc(&desc);
            gBufferCount = desc.BufferCount;

            // Create descriptor heaps
            D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
            heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            heapDesc.NumDescriptors = gBufferCount;
            if (FAILED(gDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&gHeapRTV)))) {
                return oPresentD3D12(pSwapChain, SyncInterval, Flags);
            }

            heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            if (FAILED(gDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&gHeapSRV)))) {
                return oPresentD3D12(pSwapChain, SyncInterval, Flags);
            }

            // Allocate frame contexts
            gFrameContexts = new FrameContext[gBufferCount];
            ZeroMemory(gFrameContexts, sizeof(FrameContext) * gBufferCount);

            // Create command allocator for each frame
            for (UINT i = 0; i < gBufferCount; ++i) {
                if (FAILED(gDevice->CreateCommandAllocator(
                        D3D12_COMMAND_LIST_TYPE_DIRECT,
                        IID_PPV_ARGS(&gFrameContexts[i].allocator)))) {
                    return oPresentD3D12(pSwapChain, SyncInterval, Flags);
                }
            }

            // Create RTVs for each back buffer
            UINT rtvSize = gDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            auto rtvHandle = gHeapRTV->GetCPUDescriptorHandleForHeapStart();
            for (UINT i = 0; i < gBufferCount; ++i) {
                ID3D12Resource* back;
                pSwapChain->GetBuffer(i, IID_PPV_ARGS(&back));
                gDevice->CreateRenderTargetView(back, nullptr, rtvHandle);
                gFrameContexts[i].renderTarget = back;
                gFrameContexts[i].rtvHandle = rtvHandle;
                rtvHandle.ptr += rtvSize;
            }

            // ImGui setup
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO(); (void)io;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
            ImGui::SetColorsSHF();
            ImGui_ImplWin32_Init(desc.OutputWindow);
            ImGui_ImplDX12_Init(gDevice, gBufferCount,
                desc.BufferDesc.Format,
                gHeapSRV,
                gHeapSRV->GetCPUDescriptorHandleForHeapStart(),
                gHeapSRV->GetGPUDescriptorHandleForHeapStart());

            inputhook::Init(desc.OutputWindow);

            if (!gOverlayFence) {
                if (FAILED(gDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&gOverlayFence)))) {
                    return oPresentD3D12(pSwapChain, SyncInterval, Flags);
                }
            }

            if (!gFenceEvent) {
                gFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
            }

            // Hook CommandQueue and Fence are already captured by minhook
            gInitialized = true;
        }

        if (!gShutdown) {
            // Render ImGui
            ImGui_ImplDX12_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            // CURSOR FIX
            ImGuiIO& io = ImGui::GetIO();
            io.MouseDrawCursor = GIsGUIOpen;

            if (GIsGUIOpen) menu::Init();

            UINT frameIdx = pSwapChain->GetCurrentBackBufferIndex();
            FrameContext& ctx = gFrameContexts[frameIdx];

            // Wait for the GPU to finish with the previous frame
            bool canRender = true;
            if (!gOverlayFence || !gFenceEvent) {
                // Missing synchronization objects, skip waiting
            }
            else if (gOverlayFence->GetCompletedValue() < gOverlayFenceValue) {
                HRESULT hr = gOverlayFence->SetEventOnCompletion(gOverlayFenceValue, gFenceEvent);
                if (SUCCEEDED(hr)) {
                    const DWORD waitTimeoutMs = 2000; // Extended timeout
                    DWORD waitRes = WaitForSingleObject(gFenceEvent, waitTimeoutMs);
                    if (waitRes == WAIT_TIMEOUT) {
                        canRender = false;
                    }
                    else if (waitRes != WAIT_OBJECT_0) {
                        canRender = false;
                    }
                }
                else {
                    canRender = false;
                }
            }

            if (!canRender) {
                ImGui::EndFrame();
                return oPresentD3D12(pSwapChain, SyncInterval, Flags);
            }

            // Reset allocator and command list using frame-specific allocator
            HRESULT hr = ctx.allocator->Reset();
            if (FAILED(hr)) {
                return oPresentD3D12(pSwapChain, SyncInterval, Flags);
            }

            if (!gCommandList) {
                hr = gDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                    ctx.allocator, nullptr, IID_PPV_ARGS(&gCommandList));
                if (FAILED(hr)) {
                    return oPresentD3D12(pSwapChain, SyncInterval, Flags);
                }
                gCommandList->Close();
            }
            hr = gCommandList->Reset(ctx.allocator, nullptr);
            if (FAILED(hr)) {
                return oPresentD3D12(pSwapChain, SyncInterval, Flags);
            }

            // Transition to render target
            D3D12_RESOURCE_BARRIER barrier = {};
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Transition.pResource = ctx.renderTarget;
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
            gCommandList->ResourceBarrier(1, &barrier);

            gCommandList->OMSetRenderTargets(1, &ctx.rtvHandle, FALSE, nullptr);
            ID3D12DescriptorHeap* heaps[] = { gHeapSRV };
            gCommandList->SetDescriptorHeaps(1, heaps);

            ImGui::Render();
            ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), gCommandList);

            // Transition back to present
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
            gCommandList->ResourceBarrier(1, &barrier);
            gCommandList->Close();

            // Execute
            if (gCommandQueue) {
                oExecuteCommandListsD3D12(gCommandQueue, 1, reinterpret_cast<ID3D12CommandList* const*>(&gCommandList));
                if (gOverlayFence) {
                    // Call Signal directly on the command queue to synchronize the internal overlay.
                    HRESULT hr = gCommandQueue->Signal(gOverlayFence, ++gOverlayFenceValue);
                    if (FAILED(hr)) {
                        if (gDevice) {
                            HRESULT reason = gDevice->GetDeviceRemovedReason();
                            if (reason != S_OK) {
                                release();
                            }
                        }
                    }
                }
            }
        }

        return oPresentD3D12(pSwapChain, SyncInterval, Flags);
    }

    void STDMETHODCALLTYPE HkExecuteCommandListsD3D12(ID3D12CommandQueue* _this, UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists) {
        if (!gCommandQueue && gAfterFirstPresent) {
            ID3D12Device* queueDevice = nullptr;
            if (SUCCEEDED(_this->GetDevice(__uuidof(ID3D12Device), (void**)&queueDevice))) {
                if (!gDevice) {
                    gDevice = queueDevice;
                }

                if (queueDevice == gDevice) {
                    D3D12_COMMAND_QUEUE_DESC desc = _this->GetDesc();
                    if (desc.Type == D3D12_COMMAND_LIST_TYPE_DIRECT) {
                        _this->AddRef();
                        gCommandQueue = _this;
                    }
                }

                if (queueDevice && queueDevice != gDevice) {
                    queueDevice->Release();
                }
            }
        }
        gAfterFirstPresent = false;
        oExecuteCommandListsD3D12(_this, NumCommandLists, ppCommandLists);
    }

    HRESULT STDMETHODCALLTYPE HkResizeBuffersD3D12(
        IDXGISwapChain3* pSwapChain,
        UINT BufferCount,
        UINT Width,
        UINT Height,
        DXGI_FORMAT NewFormat,
        UINT SwapChainFlags)
    {

        if (gInitialized) {
            ImGui_ImplDX12_Shutdown();
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();
            inputhook::Remove(GMainWindow);

            if (gCommandList) {
                gCommandList->Release();
                gCommandList = nullptr;
            }
            if (gHeapRTV) {
                gHeapRTV->Release();
                gHeapRTV = nullptr;
            }
            if (gHeapSRV) {
                gHeapSRV->Release();
                gHeapSRV = nullptr;
            }

            for (UINT i = 0; i < gBufferCount; ++i) {
                if (gFrameContexts[i].renderTarget) {
                    gFrameContexts[i].renderTarget->Release();
                    gFrameContexts[i].renderTarget = nullptr;
                }
                if (gFrameContexts[i].allocator) {
                    gFrameContexts[i].allocator->Release();
                    gFrameContexts[i].allocator = nullptr;
                }
            }

            delete[] gFrameContexts;
            gFrameContexts = nullptr;
            gBufferCount = 0;

            gInitialized = false;
        }

        return oResizeBuffersD3D12(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
    }

    void release() {
        gShutdown = true;
        if (GMainWindow) {
            inputhook::Remove(GMainWindow);
        }

        // Shutdown ImGui before releasing any D3D resources
        if (gInitialized && ImGui::GetCurrentContext()) {
            ImGui_ImplDX12_Shutdown();
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();
            gInitialized = false;
        }

        if (gCommandList) { gCommandList->Release(); }
        if (gHeapRTV) { gHeapRTV->Release(); }
        if (gHeapSRV) { gHeapSRV->Release(); }

        for (UINT i = 0; i < gBufferCount; ++i) {
            if (gFrameContexts[i].renderTarget) gFrameContexts[i].renderTarget->Release();
        }
        if (gOverlayFence) {
            gOverlayFence->Release();
            gOverlayFence = nullptr;
        }

        if (gFenceEvent) {
            CloseHandle(gFenceEvent);
            gFenceEvent = nullptr;
        }

        if (gCommandQueue) {
            gCommandQueue->Release();
            gCommandQueue = nullptr;
        }

        if (gDevice) gDevice->Release();
        delete[] gFrameContexts;

        // Disable hooks installed for D3D12
        imguihooks::Remove();
    }
}
