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

    Log("--- Mod MainThread Started ---");

    MH_STATUS status = MH_Initialize();
    if (status != MH_OK) {
        LogFatal("MinHook failed to initialize!");
        return 1;
    }
    Log("SUCCESS: MinHook initialized.");

    uint64_t gameBase = (uint64_t)GetModuleHandle(0);
    Log("INFO: Game base address: 0x%llX", gameBase);

    Log("INFO: Waiting for main window: '%s'", Constants::WindowName);
    while (!GMainWindow) {
        Sleep(50);
        GMainWindow = (HWND)FindWindow(0, Constants::WindowName);
    }
    Log("SUCCESS: Found main window handle: 0x%p", GMainWindow);
    
    imguihooks::Init();
    Log("INFO: ImGui hooks initialized.");

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
            Log("SUCCESS: GUObjectArray initialized.");
            Log("SUCCESS: Resolved GUObjectArray address: 0x%llX", resolvedAddress);
        }
        else {
            Log("INFO: GUObjectArray pattern not found, retrying...");
            Sleep(Constants::Time::GUObjectArrayPollIntervalMs);
        }
    }

    Log("INFO: Preparing to hook ProcessEvent...");
    void** Vft = SDK::UObject::GObjects->GetByIndex(0)->VTable;
    LPVOID** ProcessEvent = reinterpret_cast<LPVOID**>(reinterpret_cast<uintptr_t>(Vft[SDK::Offsets::ProcessEventIdx]));
    Log("INFO: Found ProcessEvent VTable entry at: 0x%p", ProcessEvent);

    status = MH_CreateHook(reinterpret_cast<LPVOID>(ProcessEvent), reinterpret_cast<void*>(Hooks::HkProcessEvent), reinterpret_cast<PVOID*>(&OProcessEvent));
    if (status != MH_OK) {
        LogFatal("Failed to create hook for ProcessEvent!");
        return 1;
    }

    status = MH_EnableHook(reinterpret_cast<LPVOID>(ProcessEvent));
    if (status != MH_OK) {
        LogFatal("Failed to enable hook for ProcessEvent!");
        return 1;
    }
    Log("SUCCESS: Hook for ProcessEvent created and enabled");

    // TODO: Clean
    Log("INFO: Scanning for FName::AppendString");
    std::vector<void*> AppendStringaddrs = scanner.Scan(Constants::PatternScan::AppendStringPattern, Constants::Hooking::AppendStringPatternSearchCount);
    Log("INFO: Found %zu potential candidates for AppendString.", AppendStringaddrs.size());
    bool appendStringFound = false;
    for (size_t i = 0; i < AppendStringaddrs.size(); ++i) {
        void* addr = AppendStringaddrs[i];
        if (!addr) { continue; }

        auto real_addr = reinterpret_cast<uintptr_t>(addr);
        Log("DEBUG: [Candidate %zu] Address: 0x%llX", real_addr);
        uintptr_t AppendStringrva = real_addr - gameBase;
        uint32_t AppendStringOffsetBase = *reinterpret_cast<uint32_t*>(real_addr + 0x3);
        uint32_t AppendStringOffset = AppendStringOffsetBase + AppendStringrva + 0x7;
        char* stringTest = reinterpret_cast<char*>(gameBase + AppendStringOffset);

        std::string cppStringTest(stringTest);
        Log("DEBUG: [Candidate %zu] Found string: '%s'", i, cppStringTest.c_str());
        if (cppStringTest == "ForwardShadingQuality_") {
            uint32_t REALAppendStringOffsetBase = *reinterpret_cast<uint32_t*>(real_addr + 35);
            uint32_t REALAppendStringOffset = REALAppendStringOffsetBase + AppendStringrva + 35 + 4;

            SDK::Offsets::AppendString = REALAppendStringOffset;
            SDK::FName::AppendString = reinterpret_cast<void*>(gameBase + REALAppendStringOffset);
            Log("SUCCESS: AppendString found! Final offset: 0x%X", REALAppendStringOffset);
            Log("AppendString Address: 0x%p", SDK::FName::AppendString);
            SDK::FName::InitInternal();
            appendStringFound = true;
            break;
        }
    }

    if(!appendStringFound){
        LogFatal("FName::AppendString could not be found!");
    }

    int testCounter = 0;
    for (int i = 0; i < SDK::UObject::GObjects->Num(); ++i) {
        auto* obj = SDK::UObject::GObjects->GetByIndex(i);
        if (obj && obj->IsValidChecked()) {
            Log("AppendString Address: 0x%p", SDK::FName::AppendString);
            std::string oName = obj->GetFullName();
            Log(oName.c_str());
            testCounter++;
        }
        if (testCounter > 10) {
            break;
        }
    }

    // TODO: Exit on fail? Probably should as it will crash anyways...
    std::vector<void*> FMemoryMallocAddrs = scanner.Scan(Constants::PatternScan::FMemoryMallocPattern);
    if (FMemoryMallocAddrs.size() == 1) {
        FMemory_Malloc = (Malloc_t)(FMemoryMallocAddrs[0]);
    }
    else {
        LogFatal("ERROR: FMemory::Malloc signature is broken!");
    }

    std::vector<void*> FMemoryFreeAddrs = scanner.Scan(Constants::PatternScan::FMemoryFreePattern);
    if (FMemoryFreeAddrs.size() == 1) {
        FMemory_Free = (Free_t)(FMemoryFreeAddrs[0]);
    }
    else {
        LogFatal("ERROR: FMemory::Free signature is broken!");
    }

    std::vector<void*> MarkRenderStateDirtyAddrs = scanner.Scan(Constants::PatternScan::MarkRenderStateDirtyPattern);
    if (MarkRenderStateDirtyAddrs.size() == 1) {
        MarkRenderStateDirty_Func = (MarkRenderStateDirty_t)(MarkRenderStateDirtyAddrs[0]);
    }
    else {
        LogFatal("ERROR: MarkRenderStateAsDirty signature is broken!");
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