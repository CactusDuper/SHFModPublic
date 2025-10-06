#pragma once

#include "SDK/BP_Pl_Hina_classes.hpp"
#include "SDK/BP_Pl_Hina_NocePlayerState_classes.hpp"
#include "SDK/BP_Pl_Hina_PlayerController_classes.hpp"
#include "SDK/GameNoce_classes.hpp"
#include <windows.h>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

extern HWND GMainWindow;
extern int GOpenGUIKey;

extern bool GIsGUIOpen;

// --- Engine Pointers & TypeDefs ---
extern SDK::UWorld* GWorld;
extern SDK::UGameEngine** GEngine;
extern SDK::ULocalPlayer* GLP;
extern SDK::USMInstance* GProgressFSM;
extern SDK::UFont* GFont;
extern SDK::UNoceGameProgressComponent* GProgressComponent;
extern SDK::ABP_Pl_Hina_NocePlayerState_C* GPlayerState;

// Define common types
using PCType = SDK::ABP_Pl_Hina_PlayerController_C;
using CharacterType = SDK::ABP_Pl_Hina_C;
using PlayerStateType = SDK::ABP_Pl_Hina_NocePlayerState_C;

// --- Hook Originals ---
typedef void(__thiscall* ProcessEvent)(SDK::UObject*, SDK::UFunction*, void*);
typedef void(__thiscall* PostRender_H)(SDK::AHUD* thisObject);

using Malloc_t = void* (*)(size_t, uint32_t);
extern Malloc_t FMemory_Malloc;

using Free_t = void (*)(void*);
extern Free_t FMemory_Free;

using MarkRenderStateDirty_t = void(__fastcall*)(SDK::UActorComponent*);
extern MarkRenderStateDirty_t MarkRenderStateDirty_Func;

extern void* GOurLastFramesBuffer;


extern PostRender_H HUDPostRenderOriginal;
extern ProcessEvent OProcessEvent;

// --- Application State ---
extern bool GIsHUDReady;
extern bool GIsInGame;


struct FCharacterDisplaySettings {
    // Master toggles
    bool bShowBaseInfo{ true };
    bool bShowLastDamage{ true };
    bool bShowEnemyInfo{ true };
    bool bShowEnemyAttributes{ true };
    bool bShowEmBaseInfo{ true };
    bool bShowComponentInfo{ true };

    // ANoceCharacter
    bool bShowCharacterName{ true };
    bool bShowCharacterTag{ true };
    bool bShowCharacterLevel{ true };
    bool bShowAliveStatus{ true };
    bool bShowDieCount{ true };
    bool bShowSignificance{ true };
    bool bShowActivatedByDistance{ true };
    bool bShowSpawner{ true };
    bool bShowTerritory{ true };
    bool bShowUnderAttackTimer{ false };
    bool bShowGameTimeSinceSpawn{ false };
    bool bShowMovementMode{ true };
    bool bShowAbilityInfo{ false };

    // Last damage
    bool bShowDamageInstigator{ true };
    bool bShowDamageCombo{ true };
    bool bShowHealthDamage{ true };
    bool bShowWinceDamage{ true };
    bool bShowWinceType{ true };

    // ANoceEnemyCharacter
    bool bShowIsFakeDead{ true };
    bool bShowUnkillable{ true };
    bool bShowForceInSight{ true };
    bool bShowHitPerformGroup{ true };
    bool bShowFakeDeadCount{ true };
    bool bShowReviveHealthRatio{ true };
    bool bShowExtendAimDistance{ true };
    bool bShowFakeDeadTimers{ false };
    bool bShowIsVisibleToPlayer{ false };

    // UNoceEnemyAttributeSet
    bool bShowHealthAttack{ true };
    bool bShowWinceAttack{ true };
    bool bShowDmgRatioHealth{ true };
    bool bShowDmgRatioWince{ true };
    bool bShowJealous{ true };
    bool bShowLinkAttackRatio{ true };
    bool bShowFakeDeadWakeupRatio{ true };

    // ABP_EmBase_NoceEnemyCharacter_C
    bool bShowIsStrafe{ true };
    bool bShowHopeNotToMove{ true };
    bool bShowEnableAIOnDamaged{ true };
    bool bShowMovementSpeeds{ true };
    bool bShowNavAgent{ true };
    bool bShowIsOptimized{ false };
    bool bShowTurnMontages{ false };
    bool bShowDefaultCapsuleInfo{ false };

    // Components
    bool bShowAIController{ true };
    bool bShowHitPerformComponent{ true };
    bool bShowAttackInfoComponent{ true };
    bool bShowDamageHandleComponent{ true };
    bool bShowBodyPartGroupComponent{ true };
    bool bShowAttackTraceComponent{ true };
    bool bShowNavLinkInfo{ true };

    // ANoceAIController
    bool bShowAIAlertType{ true };
    bool bShowAIAlertness{ true };
    bool bShowAIThinkStatus{ true };
    bool bShowAIHatestTarget{ true };
    bool bShowAIPathStatus{ false };

    // UBP_NoceHitPerformComponent_C
    bool bShowHitPerformAsset{ true };
    bool bShowHitMoveInfo{ false };

    // UNoceEnemyAttackInfoComponent
    bool bShowEnemyComboAsset{ true };
};

extern FCharacterDisplaySettings GCharDisplaySettings;

struct FCharacterSelectionInfo {
    std::wstring Name;
    bool bIsVisible = true;
};
extern std::unordered_map<uint32_t, FCharacterSelectionInfo> GCharacterSelectionMap;


extern bool GDrawShapes;
extern bool GDrawShapesNames;
extern bool GDrawCustomTriggers;
extern bool GDrawCustomTriggersNames;
extern bool GDrawAllCollisions;
extern bool GDrawCollisionNames;
extern bool GCollisionViewActive;
extern bool GDrawCharacters;
extern bool GDrawCharacterNames;
extern bool GDrawFSMInfo;
extern bool GDrawPlayerStatus;
extern bool GDrawProgressAndPlayerStateExtra;


extern bool GShouldUpdateAllCollisions;
extern bool GShouldUpdateCollisionView;
extern bool GShouldUpdateCustomTriggersVisibility;
extern bool GShouldUpdateShapesVisibility;
// TODO: Add others

extern float GDistanceToDraw;
extern SDK::FVector GPlayerLocation;

extern std::vector<uint32_t> GCharactersToDraw;
extern std::vector<uint32_t> GCustomTriggersToDraw;
extern std::vector<uint32_t> GCollisionComponentsToDraw;
extern std::vector<uint32_t> GShapesToDraw;
extern std::vector<uint32_t> GBillboardsToDraw;
extern std::vector<uint32_t> GTeleportPointsToDraw;
extern std::vector<uint32_t> GTriggersToDraw;
extern std::vector<uint32_t> GSpawnersToDraw;
extern std::vector<uint32_t> GInteractablesToDraw;
extern std::vector<uint32_t> GSavePointsToDraw;

extern std::wstring GCurrentGameStateName;
extern std::vector<std::wstring> GActiveGameplayTags;
extern std::vector<std::string> GPossibleTransitions;

struct FOriginalComponentState {
    bool bHiddenInGame;
    bool bVisible;
};

struct FOriginalActorState {
    bool bHidden;
};

extern std::unordered_map<uint32_t, FOriginalComponentState> GHiddenComponentsForCollisionView;
extern std::unordered_map<uint32_t, FOriginalComponentState> GOriginalCollisionStates;
extern std::unordered_map<uint32_t, FOriginalComponentState> GOriginalShapeVisibility;
extern std::unordered_map<uint32_t, FOriginalComponentState> GOriginalBillboardVisibility;
extern std::unordered_map<uint32_t, FOriginalActorState> GOriginalTeleportVisibility;

extern std::vector<SDK::FBatchedLine> GAllFrameLines;
constexpr size_t MAX_LINES_TO_DRAW_PER_FRAME = 500000;

extern int GCharInfoFrameCounter;
constexpr int CHAR_INFO_UPDATE_INTERVAL = 8;

struct DisplayLine {
    std::wstring label;
    std::wstring value;
    SDK::FLinearColor color;
};

struct CachedCharacterDisplayData {
    std::vector<DisplayLine> baseLines;
    std::vector<DisplayLine> enemyLines;
    std::vector<DisplayLine> emBaseLines;
    std::vector<DisplayLine> componentLines;

    CachedCharacterDisplayData() {
        baseLines.reserve(30);
        enemyLines.reserve(20);
        emBaseLines.reserve(20);
        componentLines.reserve(40);
    }
};

extern std::unordered_map<uint32_t, CachedCharacterDisplayData> GCharacterDataCache;

namespace Constants {
    namespace PatternScan {
        const std::string GUObjectArrayPattern = "74 ?? 48 8D 0D ?? ?? ?? ?? C6 05 ?? ?? ?? ?? 01 E8 ?? ?? ?? ?? C6 05 ?? ?? ?? ?? 01";
        /*
            74 ??                                   jz      short ???
            48 8D 0D ?? ?? ?? ??                    lea     rcx, ??? <- Grab this, GUObjectArray
            C6 05 ?? ?? ?? ?? 01                    mov     ???, 1
            E8 ?? ?? ?? ??                          call    ???
            C6 05 ?? ?? ?? ?? 01                    mov     ???, 1
        */
        const std::string FMemoryMallocPattern = "48 89 5C 24 08 57 48 83 EC 20 48 8B F9 8B DA 48 8B 0D ?? ?? ?? ?? 48 85 C9 75 ?? E8 ?? ?? ?? ?? 48 8B 0D";
        /*
            48 89 5C 24 08                          mov     [rsp+0x8], rbx <- Grab start of function
            57                                      push    rdi
            48 83 EC 20                             sub     rsp, 0x20h
            48 8B F9                                mov     rdi, rcx
            8B DA                                   mov     ebx, edx
            48 8B 0D ?? ?? ?? ??                    mov     rcx, ???
            48 85 C9                                test    rcx, rcx
            75 ??                                   jnz     short ???
            E8 ?? ?? ?? ??                          call    ???
            48 8B 0D ?? ?? ?? ??                    mov     rcx, ???
            48 8B 01                                mov     rax, [rcx]
            44 8B C3                                mov     r8d, ebx
            48 8B D7                                mov     rdx, rdi
            48 8B 5C 24 30                          mov     rbx, [rsp+0x30]
            48 83 C4 20                             add     rsp, 0x20
            5F                                      pop     rdi
            48 FF 60 28                             jmp     qword ptr [rax+0x28]
        */
        const std::string FMemoryFreePattern = "48 85 C9 74 ?? 53 48 83 EC 20 48 8B D9 48 8B 0D ?? ?? ?? ?? 48 85 C9 75 ?? E8 ?? ?? ?? ?? 48 8B 0D";
        /*
            48 85 C9                                test    rcx, rcx <- Grab start of function
            74 ??                                   jz      short ???
            53                                      push    rbx
            48 83 EC 20                             sub     rsp, 0x20
            48 8B D9                                mov     rbx, rcx
            48 8B 0D ?? ?? ?? ??                    mov     rcx, ???
            48 85 C9                                test    rcx, rcx
            75 ??                                   jnz     short ???
            E8 ?? ?? ?? ??                          call    ???
            48 8B 0D ?? ?? ?? ??                    mov     rcx, ???
            48 8B 01                                mov     rax, [rcx]
            48 8B D3                                mov     rdx, rbx
            FF 50 48                                call    qword ptr [rax+0x48]
            48 83 C4 20                             add     rsp, 0x20
            5B                                      pop     rbx
            C3                                      retn
        */
        const std::string MarkRenderStateDirtyPattern = "41 56 48 83 EC 20 4C 8B F1 0F B6 89 88 00 00 00 0F B6 C1 24 03 3C 03 0F 85";
        /*
            41 56                                   push    r14 <- Grab start of function
            48 83 EC 20                             sub     rsp, 0x20
            4C 8B F1                                mov     r14, rcx
            0F B6 89 88 00 00 00                    movzx   ecx, byte ptr [rcx+0x88]
            0F B6 C1                                movzx   eax, cl
            24 03                                   and     al, 3
            3C 03                                   cmp     al, 3
            0F 85 ?? ?? ?? ??                       jnz     ???
            84 C9                                   test    cl, cl
            79 ??                                   jns     short ???
            49 8B CE                                mov     rcx, r14
            E8 ?? ?? ?? ??                          call    ???
            48 85 C0                                test    rax, rax
            0F 85 ?? ?? ?? ??                       jnz     ???
            41 80 8E 88 00 00 00 80                 or      byte ptr [r14+0x88], 0x80
            48 89 5C 24 30                          mov     [rsp+0x30], rbx
            33 DB                                   xor     ebx, ebx
            48 89 6C 24 38                          mov     [rsp+0x38], rbp
            48 89 7C 24 48                          mov     [rsp+0x48], rdi
            41 38 9E 89 00 00 00                    cmp     [r14+0x89], bl
            7C ??                                   jl      short ???
            49 8B CE                                mov     rcx, r14
            E8 ?? ?? ?? ??                          call    ???
            48 8B F8                                mov     rdi, rax
            48 85 C0                                test    rax, rax
            74 ??                                   jz      short ?? ?? ?? ??
            49 8B 16                                mov     rdx, [r14]
            49 8B CE                                mov     rcx, r14
            FF 92 10 04 00 00                       call    qword ptr [rdx+0x410]
            49 8B D6                                mov     rdx, r14
            48 8B CF                                mov     rcx, rdi
            44 0F B6 C0                             movzx   r8d, al
            E8 ?? ?? ?? ??                          call    ???
            EB ??                                   jmp     short ???
            ...
        */
        const std::string AppendStringPattern = "48 8D ?? ?? ?? ?? ?? 4C 89 B4 24 50 01 00 00 48 8D 4C 24 38 E8 ?? ?? ?? ?? 48 8D 54 24 38 48 8D 4D 48 E8 ?? ?? ?? ??";
        const int AppendStringPatternOffset = 34;
        const int AppendStringPatternInstrDataLength = 5;
        /*
            48 8D ?? ?? ?? ?? ??                    lea     rdx, ??? Load string "ForwardShadingQuality_"
            4C 89 B4 24 50 01 00 00                 mov     [rsp+0x150], r14
            48 8D 4C 24 38                          lea     rcx, [rsp+0x38]
            E8 ?? ?? ?? ??                          call    ???
            48 8D 54 24 38                          lea     rdx, [rsp+0x38]
            48 8D 4D 48                             lea     rcx, [rbp+0x40]
            E8 ?? ?? ?? ??                          call    ??? <- Grab this, AppendString function
        */

    }

    namespace Time {
        constexpr int GUObjectArrayPollIntervalMs = 10;
        constexpr int GUObjectArrayPostFindDelayMs = 1000;
    }

    namespace Hooking {
        constexpr int AppendStringPatternSearchCount = 10;
    }

    static inline const char* WindowName = "SILENT HILL f  ";
}