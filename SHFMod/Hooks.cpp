#include "Hooks.hpp"

#include "Colors.hpp"
#include "Drawing.hpp"
#include "Globals.hpp"
#include "SDK/ABP_Pl_Hina_classes.hpp"
#include "SDK/Basic.hpp"
#include "SDK/BP_NoceGameState_classes.hpp"
#include "SDK/BP_Pl_Hina_classes.hpp"
#include "SDK/BP_Pl_Hina_NocePlayerState_classes.hpp"
#include "SDK/BP_PlayerTrigger_Base_classes.hpp"
#include "SDK/CoreUObject_classes.hpp"
#include "SDK/Engine_classes.hpp"
#include "SDK/Engine_structs.hpp"
#include "SDK/GameNoce_classes.hpp"
#include "SDK/GameplayTags_structs.hpp"
#include "UnrealContainers.hpp"
#include "Utils.hpp"
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <ostream>
#include <string>
#include <thread>
#include <vector>
#include <Windows.h>

void CleanupPlayer();
void TryHookHUD();


void Hooks::HkHUDPostRender(SDK::AHUD* HUD) {
    if (GIsInGame) {
        if (!HUD || !HUD->IsValidChecked() || !GLP || !GLP->IsValidChecked() || !GLP->PlayerController || !GLP->PlayerController->IsValidChecked()) {
            GIsInGame = false; // TODO: Proper cleanup
            std::cout << "UNEXPECTED POSTRENDER ISSUE!\n";
            HUDPostRenderOriginal(HUD);
            return;
        }
        GPlayerLocation = GLP->PlayerController->Character->K2_GetActorLocation();

        int ScreenWidth = HUD->Canvas->SizeX;
        int ScreenHeight = HUD->Canvas->SizeY;

        // CleanInvalidObjectIndices<SDK::UShapeComponent>(GShapesToDraw);
        // CleanInvalidObjectIndices<SDK::UBillboardComponent>(GBillboardsToDraw);
        CleanInvalidObjectIndices<SDK::ABP_PlayerTrigger_Base_C>(GCustomTriggersToDraw);
        // CleanInvalidObjectIndices<SDK::ABP_Dev_TeleportPoint_C>(GTeleportPointsToDraw);

        //if (!GShapesToDraw.empty()) { ProcessShapes(HUD, fBLUE); }
        //if (!billboardsToDraw.empty()) { processBillboards(HUD, fRED); }
        //if (!teleportPointsToDraw.empty()) { processDevTeleports(HUD, fGREEN); }

        DrawPlayerStatus(HUD);

        if (GDrawProgressAndPlayerStateExtra) {
            float yPos = 50.0f;
            float xPos = HUD->Canvas->SizeX - 450.0f; // TODO: Tweak
            HUD->Canvas->K2_DrawText(GFont, GCurrentGameStateName.c_str(), { xPos, yPos }, { 1.0f, 1.0f }, GREEN, 0, {}, {}, false, false, true, { 0,0,0,1 });
            yPos += 20;
            HUD->Canvas->K2_DrawText(GFont, L"--- Active Tags ---", { xPos, yPos }, { 1.0f, 1.0f }, RED, 0, {}, {}, false, false, true, { 0,0,0,1 });
            yPos += 20;
            for (const auto& tag : GActiveGameplayTags) {
                HUD->Canvas->K2_DrawText(GFont, SDK::FString(tag.c_str()), { xPos + 10, yPos }, { 1.0f, 1.0f }, RED, 0, {}, {}, false, false, true, { 0,0,0,1 });
                yPos += 20;
            }
        }

        HUD->Canvas->K2_DrawText(GFont, L"SFH MOD ACTIVE", { HUD->Canvas->SizeX - 120.0f, 10.0f}, {1.0f, 1.0f}, RED, 0, {}, {}, false, false, true, {0,0,0,1});

        GPossibleTransitions.clear();
        DrawFSMInfo(HUD); // Need to populate GPossibleTransitions for drawing trigger info

        ProcessCollisions(HUD);
        ProcessCharacters(HUD);
        ProcessAndDrawAllWorldLines();
        ProcessCustomTriggers(HUD, fRED);


    }

    HUDPostRenderOriginal(HUD);
}

void Hooks::HkProcessEvent(SDK::UObject* thiz, SDK::UFunction* function, void* parms) {
    // For some reason, a crash can happen while grabbing function names for some users.
    if (!thiz || !thiz->IsValidChecked() || !function) {
        OProcessEvent(thiz, function, parms);
        return;
    }

    std::string funcName = function->GetName();
    std::string fullFuncName = function->GetFullName();

    // TODO: Remove?
    if (GProgressComponent && thiz == GProgressComponent && fullFuncName == "Function SMSystem.SMStateMachineComponent.Internal_OnStateMachineStateChanged") {
        if (GProgressComponent->R_Instance) {
            SDK::FGameplayTag progressTag = GProgressComponent->GetProgressTag();
            GCurrentGameStateName = L"State: " + SDK::FString(StringToWString(progressTag.TagName.ToString()).c_str()).ToWString();
        }
    }
    else if (GPlayerState && thiz == GPlayerState && fullFuncName == "Function BP_Pl_Hina_NocePlayerState.BP_Pl_Hina_NocePlayerState_C.OnGameplayTagChange") {
        struct OnGameplayTagChange_Params {
            SDK::FGameplayTag Tag;
            bool bTagExists;
        };
        auto tagParms = static_cast<OnGameplayTagChange_Params*>(parms);

        std::wstring tagName = StringToWString(tagParms->Tag.TagName.ToString());

        if (tagParms->bTagExists) {
            bool found = false;
            // TODO: Fix
            for (const auto& tag : GActiveGameplayTags) {
                if (wcscmp(tag.c_str(), tagName.c_str()) == 0) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                GActiveGameplayTags.push_back(tagName);
            }
        }
        else {
            GActiveGameplayTags.erase(std::remove_if(GActiveGameplayTags.begin(), GActiveGameplayTags.end(),
                [&](const std::wstring& tag) {
                    return wcscmp(tag.c_str(), tagName.c_str()) == 0;
                }), GActiveGameplayTags.end());
        }
    }
    else if (funcName == "ReceiveBeginPlay") {
        if (!thiz->IsDefaultObject()) {

            if (thiz->IsA(SDK::ABP_Pl_Hina_C::StaticClass())) {
                Log("BP_Pl_Hina_C ReceiveBeginPlay called");
                SetupPlayer();
                Log("SetupPlayer done!");
            }
            else if (thiz->IsA(SDK::ABP_PlayerTrigger_Base_C::StaticClass())) { // TODO: ANocePlayerTriggerBase?
                auto* trigger = static_cast<SDK::ABP_PlayerTrigger_Base_C*>(thiz);
                GCustomTriggersToDraw.push_back(trigger->Index);
            }

            //else if (thiz->IsA(SDK::UBillboardComponent::StaticClass())) {
            //    SDK::UBillboardComponent* billboard = static_cast<SDK::UBillboardComponent*>(thiz);
            //    GBillboardsToDraw.push_back(billboard);
            //}
            //else if (thiz->IsA(SDK::ABP_AISpawnerBase_C::StaticClass())) {
            //    GSpawnersToDraw.push_back(static_cast<SDK::ABP_AISpawnerBase_C*>(thiz));
            //}
            //else if (thiz->IsA(SDK::ANoceInteractableBase::StaticClass())) {
            //    GInteractablesToDraw.push_back(static_cast<SDK::ANoceInteractableBase*>(thiz));
            //}
            //else if (thiz->IsA(SDK::ABP_RecoverIndicator_C::StaticClass())) {
            //    GSavePointsToDraw.push_back(static_cast<SDK::ABP_RecoverIndicator_C*>(thiz));
            //}

            if (thiz->IsA(SDK::UPrimitiveComponent::StaticClass())) {
                if (thiz->IsA(SDK::UShapeComponent::StaticClass())) {
                    //GShapesToDraw.push_back(thiz->Index);
                    //std::cout << "Added Shape: " << thiz->GetFullName() << ". Size: " << GShapesToDraw.size() << "\n";
                }
                else {
                    if (GDrawAllCollisions) { // TODO: UNFUCK THIS WHAT THE FUCK WAS I THINKING?
                        auto primitiveComp = static_cast<SDK::UPrimitiveComponent*>(thiz);
                        if (primitiveComp->GetCollisionEnabled() != SDK::ECollisionEnabled::NoCollision) {
                            GOriginalCollisionStates[primitiveComp->Index] = { (bool)primitiveComp->bHiddenInGame, (bool)primitiveComp->bVisible };
                            primitiveComp->SetHiddenInGame(false, false);
                            primitiveComp->SetVisibility(true, false);
                            GCollisionComponentsToDraw.push_back(primitiveComp->Index);
                        }
                    }
                }
            }

            // TODO: ABP_EmBase_NoceEnemyCharacter_C or ANoceEnemyCharacter or ANoceCharacter

            if (thiz->IsA(SDK::ANoceCharacter::StaticClass()) && thiz->IsValid()) {
                GCharactersToDraw.push_back(thiz->Index);
            }
        }
    }
    else if (funcName == "ReceiveEndPlay") {
        if (!thiz->IsDefaultObject()) {
            if (thiz->IsA(SDK::ABP_Pl_Hina_C::StaticClass())) {
                Log("BP_Pl_Hina_C ReceiveEndPlay called");
                CleanupPlayer();
            }
            if (thiz->IsA(SDK::ABP_PlayerTrigger_Base_C::StaticClass())) {
                auto* trigger = static_cast<SDK::ABP_PlayerTrigger_Base_C*>(thiz);
                std::erase(GCustomTriggersToDraw, trigger->Index);
            }
            if (thiz->IsA(SDK::UPrimitiveComponent::StaticClass())) {
                std::erase(GCollisionComponentsToDraw, thiz->Index);
                //std::erase(GShapesToDraw, thiz->Index);
            }
            if (thiz->IsA(SDK::ANoceCharacter::StaticClass())) {
                std::erase(GCharactersToDraw, thiz->Index);
            }
            //if (thiz->IsA(SDK::ABP_AISpawnerBase_C::StaticClass())) {
            //    std::erase(spawnersToDraw, static_cast<SDK::ABP_AISpawnerBase_C*>(thiz));
            //}
            //else if (thiz->IsA(SDK::ANoceInteractableBase::StaticClass())) {
            //    std::erase(interactablesToDraw, static_cast<SDK::ANoceInteractableBase*>(thiz));
            //}
            //else if (thiz->IsA(SDK::ABP_RecoverIndicator_C::StaticClass())) {
            //    std::erase(savePointsToDraw, static_cast<SDK::ABP_RecoverIndicator_C*>(thiz));
            //}
        }
    }

    //if (thiz->IsA(SDK::UActorComponent::StaticClass()) || thiz->IsA(SDK::AActor::StaticClass())) {
    //    if (thiz->GetFullName() != "BP_Fogsheet_ProceduralRamp_C") {
    //        std::cout << thiz->GetFullName() << " " << function->GetFullName() << "\n";
    //    }
    //}

    OProcessEvent(thiz, function, parms);
}

void SetupPlayer() {
    GIsInGame = true;
    if (GFont == nullptr) {
        GFont = SDK::UObject::FindObject<SDK::UFont>("Font Roboto.Roboto");
    }
    Log("Found font!");

    GWorld = SDK::UWorld::GetWorld();
    if (!GWorld) {
        LogFatal("Unable to find GWorld!\n");
        return;
    }

    if (GWorld && GWorld->OwningGameInstance && GWorld->OwningGameInstance->LocalPlayers[0] && GWorld->OwningGameInstance->LocalPlayers[0]->PlayerController) {
        auto PController = GWorld->OwningGameInstance->LocalPlayers[0]->PlayerController;
        Log("Found PlayerController!");
        GLP = GWorld->OwningGameInstance->LocalPlayers[0];
        TryHookHUD();
    }
    else {
        Log("Something went wrong when trying to get PlayerController!");
    }


    if (GWorld && GWorld->GameState && GWorld->GameState->IsA(SDK::ABP_NoceGameState_C::StaticClass())) {
        auto gState = static_cast<SDK::ABP_NoceGameState_C*>(GWorld->GameState);
        GProgressComponent = gState->GameProgress;
        Log("Found NoceGameProgressComponent!");

        if (GProgressComponent && GProgressComponent->R_Instance) {
            GProgressFSM = GProgressComponent->R_Instance;
            Log("Successfully found Chapter FSM Instance!");
        }
        else {
            Log("Could not find R_Instance on NoceGameProgressComponent!");
        }
    }
    else {
        Log("Something went wrong trying to get GameState/ProgressCompoenent!");
    }

    if (GLP && GLP->PlayerController && GLP->PlayerController->PlayerState) {
        GPlayerState = static_cast<SDK::ABP_Pl_Hina_NocePlayerState_C*>(GLP->PlayerController->PlayerState);
        Log("Found PlayerState!");
    }
    else {
        Log("Unable to get PlayerState!");
    }
    CheckWorldForCustomTriggers(); // Initial scan for triggers
    Log("Scanned for triggers!");
}

void TryHookHUD() {
    if (GLP && GLP->PlayerController && GLP->PlayerController->Character && GLP->PlayerController->Character->IsA(CharacterType::StaticClass())) {
        Log("trying to hook hud");
        CharacterType* player = static_cast<CharacterType*>(GLP->PlayerController->Character);

        SDK::APlayerState* playerState = static_cast<SDK::APlayerState*>(player->PlayerState);
        if (playerState) {

            if (!GIsHUDReady) {
                if (GLP->PlayerController->MyHUD->IsA(SDK::AHUD::StaticClass())) {
                    auto hud = static_cast<SDK::AHUD*>(GLP->PlayerController->MyHUD);
                    void** HUDVFTable = hud->VTable;
                    DWORD HUDProtect = 0;
                    VirtualProtect(&HUDVFTable[SDK::Offsets::HUDPostRenderIdx], 8, PAGE_EXECUTE_READWRITE, &HUDProtect);
                    HUDPostRenderOriginal = reinterpret_cast<decltype(HUDPostRenderOriginal)>(HUDVFTable[SDK::Offsets::HUDPostRenderIdx]);
                    HUDVFTable[SDK::Offsets::HUDPostRenderIdx] = &Hooks::HkHUDPostRender;
                    VirtualProtect(&HUDVFTable[SDK::Offsets::HUDPostRenderIdx], 8, HUDProtect, 0);

                    GIsHUDReady = true;

                    Log("HUD hook is done");
                }
            }
        }
    }
}

// TODO: Cleanup everything missing from this
void CleanupPlayer() {
    Log("Cleaning up info!");
    GIsInGame = false;

    GLP = nullptr;
    GProgressFSM = nullptr;
    GProgressComponent = nullptr;
    GPlayerState = nullptr;

    GCharactersToDraw.clear();
    GCollisionComponentsToDraw.clear();
    GShapesToDraw.clear();
    GBillboardsToDraw.clear();
    GCustomTriggersToDraw.clear();
    GTeleportPointsToDraw.clear();
    GTriggersToDraw.clear();
    GSpawnersToDraw.clear();
    GInteractablesToDraw.clear();
    GSavePointsToDraw.clear();

    GHiddenComponentsForCollisionView.clear();
    GOriginalCollisionStates.clear();
    GOriginalShapeVisibility.clear();
    GOriginalBillboardVisibility.clear();
    GOriginalTeleportVisibility.clear();

    GActiveGameplayTags.clear();
    GCurrentGameStateName = L"State: Unknown";
    Log("Finished cleaning up info!");
}