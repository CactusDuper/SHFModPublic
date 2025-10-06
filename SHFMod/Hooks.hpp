#pragma once

#include "SDK/CoreUObject_classes.hpp"
#include "SDK/Engine_classes.hpp"

namespace Hooks {
    void Initialize();
    void Shutdown();

    // Hooked function declarations
    void HkProcessEvent(SDK::UObject* thiz, SDK::UFunction* function, void* parms);
    void HkHUDPostRender(SDK::AHUD* HUD);
}

void SetupPlayer();