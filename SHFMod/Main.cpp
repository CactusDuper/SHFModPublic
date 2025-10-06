#include "MinHook.h"
#include "SDK/Basic.hpp"
#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>
#include <vector>
#include <Windows.h>
#include "Hooks.hpp"
#include "Globals.hpp"
#include "SignatureScanner.hpp"

#include "stdafx.h"


DWORD MainThread(HMODULE Module) {
    AllocConsole();
    FILE* Dummy;
    freopen_s(&Dummy, "CONOUT$", "w", stdout);
    freopen_s(&Dummy, "CONIN$", "r", stdin);

    MH_Initialize();

    uint64_t gameBase = (uint64_t)GetModuleHandle(0);

    while (!GMainWindow) {
        Sleep(50);
        GMainWindow = (HWND)FindWindow(0, Constants::WindowName);
    }
    imguihooks::Init();

    // TODO: Properly scan for everything at once
    bool foundGObjects = false;
    SignatureScanner scanner;

    while (!foundGObjects) {
        uintptr_t resolvedAddress = scanner.ResolveRipRelativeAddress(Constants::PatternScan::GUObjectArrayPattern, 2, 7); // Skip jz, grab from lea (see GUObjectArrayPattern comment)

        if (resolvedAddress != 0) {
            void* manualInit = (void*)(resolvedAddress + 0x10);
            SDK::Offsets::GObjects = resolvedAddress + 0x10 - gameBase;
            SDK::UObject::GObjects.InitManually(manualInit);
            foundGObjects = true;
        }
        else {
            Sleep(Constants::Time::GUObjectArrayPollIntervalMs);
        }
    }


    void** Vft = SDK::UObject::GObjects->GetByIndex(0)->VTable;
    LPVOID** ProcessEvent = reinterpret_cast<LPVOID**>(reinterpret_cast<uintptr_t>(Vft[SDK::Offsets::ProcessEventIdx]));

    MH_CreateHook(reinterpret_cast<LPVOID>(ProcessEvent), reinterpret_cast<void*>(Hooks::HkProcessEvent), reinterpret_cast<PVOID*>(&OProcessEvent));
    MH_EnableHook(reinterpret_cast<LPVOID>(ProcessEvent));

    // TODO: Clean
    std::vector<void*> AppendStringaddrs = scanner.Scan(Constants::PatternScan::AppendStringPattern, Constants::Hooking::AppendStringPatternSearchCount);
    bool appendStringFound = false;
    for (auto addr : AppendStringaddrs) {
        if (!addr) { continue; }
        auto real_addr = reinterpret_cast<uintptr_t>(addr);
        uintptr_t AppendStringrva = real_addr - gameBase;
        uint32_t AppendStringOffsetBase = *reinterpret_cast<uint32_t*>(real_addr + 0x3);
        uint32_t AppendStringOffset = AppendStringOffsetBase + AppendStringrva + 0x7;
        char* stringTest = reinterpret_cast<char*>(gameBase + AppendStringOffset);

        std::string cppStringTest(stringTest);
        if (cppStringTest == "ForwardShadingQuality_") {
            uint32_t REALAppendStringOffsetBase = *reinterpret_cast<uint32_t*>(real_addr + 35);
            uint32_t REALAppendStringOffset = REALAppendStringOffsetBase + AppendStringrva + 35 + 4;

            SDK::Offsets::AppendString = REALAppendStringOffset;
            SDK::FName::AppendString = reinterpret_cast<void*>(gameBase + REALAppendStringOffset);
            appendStringFound = true;
            break;
        }
    }

    // TODO: Exit on fail? Probably should as it will crash anyways...
    std::vector<void*> FMemoryMallocAddrs = scanner.Scan(Constants::PatternScan::FMemoryMallocPattern);
    if (FMemoryMallocAddrs.size() == 1) {
        FMemory_Malloc = (Malloc_t)(FMemoryMallocAddrs[0]);
    }
    else {
        std::cout << "ERROR: FMemory::Malloc signature is broken!\n";
    }

    std::vector<void*> FMemoryFreeAddrs = scanner.Scan(Constants::PatternScan::FMemoryFreePattern);
    if (FMemoryFreeAddrs.size() == 1) {
        FMemory_Free = (Free_t)(FMemoryFreeAddrs[0]);
    }
    else {
        std::cout << "ERROR: FMemory::Free signature is broken!\n";
    }

    std::vector<void*> MarkRenderStateDirtyAddrs = scanner.Scan(Constants::PatternScan::MarkRenderStateDirtyPattern);
    if (MarkRenderStateDirtyAddrs.size() == 1) {
        MarkRenderStateDirty_Func = (MarkRenderStateDirty_t)(MarkRenderStateDirtyAddrs[0]);
    }
    else {
        std::cout << "ERROR: MarkRenderStateAsDirty signature is broken!\n";
    }


    std::cout << "Mod loaded successfully. Press DELETE to open the menu." << std::endl;

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    switch (reason) {
        case DLL_PROCESS_ATTACH:
            CreateThread(0, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, 0);
            break;
        case DLL_PROCESS_DETACH:
            break;
    }

    return TRUE;
}