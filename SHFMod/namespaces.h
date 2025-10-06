// Credit: https://github.com/Sh0ckFR/Universal-Dear-ImGui-Hook
#pragma once

namespace imguihooks {
        extern void Init();
}

namespace inputhook {
        extern void Init(HWND hWindow);
        extern void Remove(HWND hWindow);
        static LRESULT APIENTRY WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
}

namespace d3d12hook {
        typedef HRESULT(STDMETHODCALLTYPE* PresentD3D12)(IDXGISwapChain3 * pSwapChain, UINT SyncInterval, UINT Flags);
        extern PresentD3D12 oPresentD3D12;
        extern long __fastcall HkPresentD3D12(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags);

	    typedef void(STDMETHODCALLTYPE* ExecuteCommandListsFn)(ID3D12CommandQueue * _this, UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists);
        extern ExecuteCommandListsFn oExecuteCommandListsD3D12;
        extern void STDMETHODCALLTYPE HkExecuteCommandListsD3D12(ID3D12CommandQueue* _this, UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists);

        typedef HRESULT(STDMETHODCALLTYPE* ResizeBuffersFn)(IDXGISwapChain3* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
        extern ResizeBuffersFn oResizeBuffersD3D12;
        extern HRESULT STDMETHODCALLTYPE HkResizeBuffersD3D12(IDXGISwapChain3* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);

        extern void release();
}

namespace menu {
        extern void Init();
}