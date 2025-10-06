#include "Drawing.hpp"

#include "Colors.hpp"
#include "Globals.hpp"
#include "SDK/BP_AddRemoveMapIconTrigger_classes.hpp"
#include "SDK/BP_AISpawnerEventTrigger_classes.hpp"
#include "SDK/BP_AutoSave_PlayerTrigger_classes.hpp"
#include "SDK/BP_EmBase_NoceEnemyCharacter_classes.hpp"
#include "SDK/BP_PlayDialogTrigger_classes.hpp"
#include "SDK/BP_PlayerTrigger_Base_classes.hpp"
#include "SDK/BP_NoceHitPerformComponent_classes.hpp"
#include "SDK/BPC_EmBase_NavLinkInfo_classes.hpp"
#include "Utils.hpp"
#include <Windows.h>
#include <algorithm>
#include "Utils.cpp"
#include <unordered_set>
#include <chrono>

#undef min // Fuck you windows


void DrawTextWithOffset(SDK::AHUD* HUD, const SDK::FString& Text, SDK::FVector2D BasePosition, float& YOffset, SDK::FLinearColor Color) {
    if (!HUD || !HUD->Canvas || !GFont) {
        return;
    }

    SDK::FVector2D TextPosition = BasePosition;
    TextPosition.Y += YOffset;

	const SDK::FVector2D Scale = { 1.0f, 1.0f };
	const SDK::FLinearColor ShadowColor = { 0.0f, 0.0f, 0.0f, 0.7f };
	const SDK::FVector2D ShadowOffset = { 1.0f, 1.0f };
	const SDK::FLinearColor OutlineColor = { 0.0f, 0.0f, 0.0f, 1.0f };
    HUD->Canvas->K2_DrawText(GFont, Text, TextPosition, Scale, Color, 0.0f, ShadowColor, ShadowOffset, false, false, true, OutlineColor);

    float textWidth, textHeight;
    HUD->GetTextSize(Text, &textWidth, &textHeight, GFont, 1.0f);
    YOffset += textHeight;
}

void DrawTextColumn(SDK::AHUD* HUD, const std::vector<DisplayLine>& lines, float& currentX, float startY, float columnWidth) {
    if (!HUD || lines.empty()) {
        currentX += columnWidth;
        return;
    }

    float yOffset = startY;
    for (const auto& line : lines) {
        SDK::FVector2D columnStartPos = { currentX, 0.0f };
        DrawTextWithOffset(HUD, (line.label + line.value).c_str(), columnStartPos, yOffset, line.color);
    }

    currentX += columnWidth;
}

// TODO: REMOVE
void DrawBoxAroundSceneComponentHUD(SDK::UStaticMeshComponent* staticMesh, SDK::AHUD* HUD, SDK::FLinearColor BoxColor, float LineThickness) {
    if (!staticMesh || !HUD || !HUD->Canvas) { 
        return;
    }

    SDK::UStaticMesh* mesh = staticMesh->StaticMesh;
    if (!mesh) {
        return;
    }

    SDK::FVector minBounds, maxBounds;
    staticMesh->GetLocalBounds(&minBounds, &maxBounds);
    SDK::FTransform componentToWorld = staticMesh->K2_GetComponentToWorld();

    SDK::FVector corners[8];
    corners[0] = componentToWorld.TransformPosition(minBounds); // Min, Min, Min
    corners[1] = componentToWorld.TransformPosition(SDK::FVector(maxBounds.X, minBounds.Y, minBounds.Z)); // Max, Min, Min
    corners[2] = componentToWorld.TransformPosition(SDK::FVector(maxBounds.X, maxBounds.Y, minBounds.Z)); // Max, Max, Min
    corners[3] = componentToWorld.TransformPosition(SDK::FVector(minBounds.X, maxBounds.Y, minBounds.Z)); // Min, Max, Min

    corners[4] = componentToWorld.TransformPosition(SDK::FVector(minBounds.X, minBounds.Y, maxBounds.Z)); // Min, Min, Max
    corners[5] = componentToWorld.TransformPosition(SDK::FVector(maxBounds.X, minBounds.Y, maxBounds.Z)); // Max, Min, Max
    corners[6] = componentToWorld.TransformPosition(maxBounds); // Max, Max, Max
    corners[7] = componentToWorld.TransformPosition(SDK::FVector(minBounds.X, maxBounds.Y, maxBounds.Z)); // Min, Max, Max

    SDK::FVector2D projectedCorners[8];
    bool bIsOnScreen[8];
    for (int i = 0; i < 8; ++i) {
        SDK::FVector2D screenPos;
        SDK::FVector projectResult = HUD->Project(corners[i], false);
        screenPos.X = projectResult.X;
        screenPos.Y = projectResult.Y;
        projectedCorners[i] = screenPos;

        bool isYOnScreen = projectResult.Y > 0 && projectResult.Y < HUD->Canvas->ClipY;
        bool isXOnScreen = projectResult.X > 0 && projectResult.X < HUD->Canvas->ClipX;

        bIsOnScreen[i] = projectResult.Z > 0 && projectResult.Z < 1 && isYOnScreen && isXOnScreen; // Prevent clipping
    }

    bool bAnyCornerOnScreen = false;
    for (bool onScreen : bIsOnScreen) {
        if (onScreen) {
            bAnyCornerOnScreen = true;
            break;
        }
    }
    //if (!bAnyCornerOnScreen) return;


    // Bottom face
    HUD->DrawLine(projectedCorners[0].X, projectedCorners[0].Y, projectedCorners[1].X, projectedCorners[1].Y, BoxColor, LineThickness);
    HUD->DrawLine(projectedCorners[1].X, projectedCorners[1].Y, projectedCorners[2].X, projectedCorners[2].Y, BoxColor, LineThickness);
    HUD->DrawLine(projectedCorners[2].X, projectedCorners[2].Y, projectedCorners[3].X, projectedCorners[3].Y, BoxColor, LineThickness);
    HUD->DrawLine(projectedCorners[3].X, projectedCorners[3].Y, projectedCorners[0].X, projectedCorners[0].Y, BoxColor, LineThickness);

    // Top face
    HUD->DrawLine(projectedCorners[4].X, projectedCorners[4].Y, projectedCorners[5].X, projectedCorners[5].Y, BoxColor, LineThickness);
    HUD->DrawLine(projectedCorners[5].X, projectedCorners[5].Y, projectedCorners[6].X, projectedCorners[6].Y, BoxColor, LineThickness);
    HUD->DrawLine(projectedCorners[6].X, projectedCorners[6].Y, projectedCorners[7].X, projectedCorners[7].Y, BoxColor, LineThickness);
    HUD->DrawLine(projectedCorners[7].X, projectedCorners[7].Y, projectedCorners[4].X, projectedCorners[4].Y, BoxColor, LineThickness);

    // Vertical lines
    HUD->DrawLine(projectedCorners[0].X, projectedCorners[0].Y, projectedCorners[4].X, projectedCorners[4].Y, BoxColor, LineThickness);
    HUD->DrawLine(projectedCorners[1].X, projectedCorners[1].Y, projectedCorners[5].X, projectedCorners[5].Y, BoxColor, LineThickness);
    HUD->DrawLine(projectedCorners[2].X, projectedCorners[2].Y, projectedCorners[6].X, projectedCorners[6].Y, BoxColor, LineThickness);
    HUD->DrawLine(projectedCorners[3].X, projectedCorners[3].Y, projectedCorners[7].X, projectedCorners[7].Y, BoxColor, LineThickness);

}

bool GetShapeLocalBounds(SDK::UPrimitiveComponent* primitiveComp, SDK::FVector& outMinBounds, SDK::FVector& outMaxBounds) {
    if (primitiveComp->IsA(SDK::UBoxComponent::StaticClass())) {
        auto boxComp = static_cast<SDK::UBoxComponent*>(primitiveComp);
        SDK::FVector extent = boxComp->BoxExtent;
        outMinBounds = { -extent.X, -extent.Y, -extent.Z };
        outMaxBounds = extent;
        return true;
    }

    if (primitiveComp->IsA(SDK::UCapsuleComponent::StaticClass())) {
        if (primitiveComp->IsA(SDK::UCapsuleComponent::StaticClass())) {
            auto capsuleComp = static_cast<SDK::UCapsuleComponent*>(primitiveComp);
            double radius = capsuleComp->CapsuleRadius;
            double halfHeight = capsuleComp->CapsuleHalfHeight;

            // Bounding box of a capsule
            outMinBounds = { (float)-radius, (float)-radius, (float)-halfHeight };
            outMaxBounds = { (float)radius, (float)radius, (float)halfHeight };
            return true;
        }
    }

    if (primitiveComp->IsA(SDK::USphereComponent::StaticClass())) {
        auto sphereComp = static_cast<SDK::USphereComponent*>(primitiveComp);
        double radius = sphereComp->SphereRadius;
        outMinBounds = { (float)-radius, (float)-radius, (float)-radius };
        outMaxBounds = { (float)radius, (float)radius, (float)radius };
        return true;
    }

    // TODO: Handle other shape types?
	std::cout << "Unsupported shape type: " << primitiveComp->GetFullName() << "\n";
    return false;
}

void DrawPlayerStatus(SDK::AHUD* HUD) {
    if (!GDrawPlayerStatus || !HUD || !HUD->Canvas || !GLP || !GLP->PlayerController || !GLP->PlayerController->Character || !GPlayerState) {
        return;
    }

    CharacterType* Character = static_cast<CharacterType*>(GLP->PlayerController->Character);
    PlayerStateType* PS = GPlayerState;

    if (!Character->IsValidChecked() || !PS->IsValidChecked()) {
        return;
    }

    std::vector<DisplayLine> lines;

    const float COLUMN_WIDTH = 400.0f;
    float startX = HUD->Canvas->SizeX - COLUMN_WIDTH;
    float currentY = HUD->Canvas->SizeY / 2.0f - 150.0f;

    auto AddLine = [&](std::wstring l, std::wstring v, const SDK::FLinearColor& c) {
        lines.push_back({ l, v, c });
        };

    const SDK::FLinearColor BoolTrueColor = { 0.2f, 1.0f, 0.2f, 1.0f };
    const SDK::FLinearColor BoolFalseColor = GRAY;
    const SDK::FLinearColor HeaderColor = { 0.2f, 0.8f, 1.0f, 1.0f };

    AddLine(L"--- PLAYER ATTRIBUTES ---", L"", HeaderColor);

    float HealthRatio = PS->HealthRatio;
    AddLine(L"Health Ratio: ", std::to_wstring(HealthRatio), (HealthRatio < 0.25f ? RED : (HealthRatio < 0.5f ? YELLOW : LIME)));

    float StaminaRatio = PS->StaminaRatio;
    AddLine(L"Stamina Ratio: ", std::to_wstring(StaminaRatio), (StaminaRatio < 0.1f ? ORANGE : LIGHT_BLUE));

    float SanityRatio = PS->SanityRatio;
    AddLine(L"Sanity Ratio: ", std::to_wstring(SanityRatio), SanityRatio < 0.2f ? RED : PURPLE);

    float CurrentMaxSanityRatio = PS->CurrentMaxSanityRatio;
    AddLine(L"Max Sanity Ratio: ", std::to_wstring(CurrentMaxSanityRatio), YELLOW);

    float ClawTransformRatio = PS->ClawTransformRatio;
    AddLine(L"Claw Ratio: ", std::to_wstring(ClawTransformRatio), TEAL);

    AddLine(L"", L"", BaseColor);

    AddLine(L"--- MOVEMENT ---", L"", HeaderColor);

    SDK::FVector Loc = Character->K2_GetActorLocation();
    AddLine(L"Location: ", VectorToWString(Loc), BaseColor);

    SDK::FRotator Rot = Character->K2_GetActorRotation(); // TODO: Pull from CharacterMovement instead?
    AddLine(L"Control Rot: ", RotatorToWString(GLP->PlayerController->ControlRotation), GRAY);
    AddLine(L"Actor Rotation: ", RotatorToWString(Rot), BaseColor);

    SDK::FVector Velocity = Character->GetVelocity(); // TODO: Pull from CharacterMovement instead?
    float Speed = Velocity.Magnitude();
    AddLine(L"Speed: ", std::to_wstring((int)Speed) + L" u/s", Speed > 500.0f ? LIME : BaseColor);

    SDK::UCharacterMovementComponent* Movement = Character->CharacterMovement;
    if (Movement) {
        AddLine(L"Acceleration: ", VectorToWString(Movement->Acceleration), BaseColor);
    }

    for (const auto& line : lines) {
        DrawTextWithOffset(HUD, (line.label + line.value).c_str(), { startX, 0.0f }, currentY, line.color);
    }
}

void DrawFSMHierarchyRecursive(SDK::AHUD* HUD, SDK::FSMStateMachine* StateMachine, float& X, float& Y, std::wstring Indent) {
    if (!StateMachine) {
        return;
    }

    for (int i = 0; i < StateMachine->ActiveStates.Num(); ++i) {
        SDK::FSMState_Base* ActiveState = StateMachine->ActiveStates[i];
        if (!ActiveState) {
            continue;
        }

        std::string stateName = ActiveState->NodeName.ToString();
        float timeInState = ActiveState->TimeInState;

        std::wstring stateText = Indent + L"State: " + StringToWString(stateName);
        DrawTextWithOffset(HUD, stateText.c_str(), { X, 0.0f }, Y, GREEN);

        std::wstring timeText = Indent + L" Time: " + std::to_wstring(timeInState);
        DrawTextWithOffset(HUD, timeText.c_str(), { X, 0.0f }, Y, { 1.f, 1.f, 0.f, 1.f });

        if (ActiveState->OutgoingTransitions.Num() > 0) {
            DrawTextWithOffset(HUD, (Indent + L"  Possible Transitions:").c_str(), { X, 0.0f }, Y, { 0.7f, 0.7f, 1.0f, 1.0f });
            for (int j = 0; j < ActiveState->OutgoingTransitions.Num(); ++j) {
                SDK::FSMTransition* transition = ActiveState->OutgoingTransitions[j];
                if (transition && transition->ToState) {
                    std::string toStateName = transition->ToState->NodeName.ToString();
                    DrawTextWithOffset(HUD, (Indent + L"    -> " + StringToWString(toStateName)).c_str(), { X, 0.0f }, Y, { 0.7f, 0.7f, 1.0f, 1.0f });

                    GPossibleTransitions.push_back(toStateName);
                }
            }
        }

        if (ActiveState->NodeInstance && ActiveState->NodeInstance->IsA(SDK::USMStateMachineInstance::StaticClass())) {
            auto subStateMachineStruct = static_cast<SDK::FSMStateMachine*>(ActiveState);
            if (subStateMachineStruct->ReferencedStateMachine) {
                SDK::FSMStateMachine* nextMachineToProcess = &subStateMachineStruct->ReferencedStateMachine->RootStateMachine;
                DrawFSMHierarchyRecursive(HUD, nextMachineToProcess, X, Y, Indent + L"  ");
            }
        }
    }
}

void DrawFSMInfo(SDK::AHUD* HUD) {
    if (!GDrawFSMInfo || !GProgressFSM || !GProgressFSM->IsValid()) {
        return;
    }

    float yPos = 50.0f;
    float xPos = 50.0f;

    DrawTextWithOffset(HUD, L"--- Game Progress FSM ---", { xPos, 0.0f }, yPos, GREEN);

    std::string fsmClassName = GProgressFSM->GetFullName();
    DrawTextWithOffset(HUD, StringToWString("Entry FSM: " + fsmClassName).c_str(), { xPos, 0.0f }, yPos, { 0.8f, 0.8f, 0.8f, 1.f });

    yPos += 5.f;
    HUD->DrawLine(xPos, yPos, xPos + 400, yPos, { 0.5f, 0.5f, 0.5f, 1.f }, 1.f);
    yPos += 5.f;

    DrawFSMHierarchyRecursive(HUD, &GProgressFSM->RootStateMachine, xPos, yPos, L"");
}

void UpdateCollisionView() {
    if (GCollisionViewActive) {
        std::cout << "Activating Collision View: Hiding all primitive components..." << std::endl;
        GHiddenComponentsForCollisionView.clear();

        for (int i = 0; i < SDK::UObject::GObjects->Num(); ++i) {
            SDK::UObject* obj = SDK::UObject::GObjects->GetByIndex(i);

            if (!obj || obj->IsDefaultObject() || !obj->IsA(SDK::UPrimitiveComponent::StaticClass())) {
                continue;
            }

            auto comp = static_cast<SDK::UPrimitiveComponent*>(obj);

            // TODO: See if removing this will show any extra things that get missed. It shouldn't...
            if (!comp->WorldPrivate) {
                continue;
            }

            auto owner = comp->OwnerPrivate;
            if (owner && (owner == GLP->PlayerController || owner == GLP->PlayerController->Character)) {
                continue;
            }

            if (obj->IsA(SDK::UStaticMeshComponent::StaticClass())) {
                auto sm = static_cast<SDK::UStaticMeshComponent*>(obj);

                if (sm->StaticMesh) {
                    std::string meshName = sm->StaticMesh->GetFullName();
                    if (meshName.find("_BmCameraHit_BCol") != std::string::npos) { // TODO: Add more
                        continue;
                    }
                }
            }

			// Do not hide components that are explicitly marked for collision drawing
            bool isExplicitlyDrawnCollision = std::find(GCollisionComponentsToDraw.begin(), GCollisionComponentsToDraw.end(), comp->Index) != GCollisionComponentsToDraw.end();
            if (!isExplicitlyDrawnCollision) {
                GHiddenComponentsForCollisionView[i] = { (bool)comp->bHiddenInGame, (bool)comp->bVisible };
                comp->SetVisibility(false, false);
                comp->SetHiddenInGame(true, false);
            }
        }
        std::cout << "Hid " << GHiddenComponentsForCollisionView.size() << " components for Collision View." << std::endl;
    }
    else {
        std::cout << "Deactivating Collision View: Restoring component visibility..." << std::endl;
        for (auto const& [index, originalState] : GHiddenComponentsForCollisionView) {
            SDK::UObject* obj = SDK::UObject::GObjects->GetByIndex(index);

            if (!obj || obj->IsDefaultObject() || !obj->IsA(SDK::UPrimitiveComponent::StaticClass())) {
                continue;
            }
            auto comp = static_cast<SDK::UPrimitiveComponent*>(obj);
            comp->SetVisibility(originalState.bVisible, false);
            comp->SetHiddenInGame(originalState.bHiddenInGame, false);
        }
        GHiddenComponentsForCollisionView.clear();
        std::cout << "Restored visibility for components." << std::endl;
    }

    GShouldUpdateCollisionView = false;
}

void UpdateAllCollisionComponentsList() {
    if (!GWorld) {
        return;
    }

	auto currentSize = GCollisionComponentsToDraw.size();
    GCollisionComponentsToDraw.clear();
	GCollisionComponentsToDraw.reserve(currentSize);

    for (int i = 0; i < SDK::UObject::GObjects->Num(); ++i) {
        auto obj = SDK::UObject::GObjects->GetByIndex(i);
        if (obj && !obj->IsDefaultObject() && obj->IsValidChecked() && obj->IsA(SDK::UPrimitiveComponent::StaticClass()) && !obj->IsA(SDK::UShapeComponent::StaticClass())) {
            auto primitiveComp = static_cast<SDK::UPrimitiveComponent*>(obj);
            if (primitiveComp->WorldPrivate && primitiveComp->GetCollisionEnabled() != SDK::ECollisionEnabled::NoCollision) {
                GCollisionComponentsToDraw.push_back(i);
            }
        }
    }
    std::cout << "Found " << GCollisionComponentsToDraw.size() << " collision components after full scan." << std::endl;
}


void ProcessCollisions(SDK::AHUD* HUD) {
    if (GShouldUpdateCollisionView) {
        UpdateCollisionView();
    }

	// TODO: Change this. What was I thinking???
    if (GShouldUpdateAllCollisions) {
        if (GDrawAllCollisions) {
            std::cout << "Enabling collision visualization..." << std::endl;
            GOriginalCollisionStates.clear();
			UpdateAllCollisionComponentsList(); // Scan for all collision components

            for (auto index : GCollisionComponentsToDraw) {
                SDK::UObject* obj = SDK::UObject::GObjects->GetByIndex(index);

                if (!obj || obj->IsDefaultObject() || !obj->IsA(SDK::UPrimitiveComponent::StaticClass())) {
                    continue;
                }
                auto comp = static_cast<SDK::UPrimitiveComponent*>(obj);
                GOriginalCollisionStates[index] = { (bool)comp->bHiddenInGame, (bool)comp->bVisible };
                comp->SetHiddenInGame(false, false);
                comp->SetVisibility(true, false);
            }
        }
        else {
            std::cout << "Disabling collision visualization..." << std::endl;
            for (auto const& [index, originalState] : GOriginalCollisionStates) {
                SDK::UObject* obj = SDK::UObject::GObjects->GetByIndex(index);

                if (!obj || obj->IsDefaultObject() || !obj->IsA(SDK::UPrimitiveComponent::StaticClass())) {
                    continue;
                }
                auto comp = static_cast<SDK::UPrimitiveComponent*>(obj);
                comp->SetHiddenInGame(originalState.bHiddenInGame, false);
                comp->SetVisibility(originalState.bVisible, false);
            }
            GOriginalCollisionStates.clear();
            GCollisionComponentsToDraw.clear();
        }
        GShouldUpdateAllCollisions = false;
    }

    if (!GDrawAllCollisions || !GDrawCollisionNames || !HUD || !HUD->Canvas) {
        return;
    }

    // Kinda want to delete this...
    for (const auto& index : GCollisionComponentsToDraw) {
        SDK::UObject* obj = SDK::UObject::GObjects->GetByIndex(index);
        if (!obj->IsValid() || !obj->IsA(SDK::UPrimitiveComponent::StaticClass())) {
            continue;
        }

        auto comp = static_cast<SDK::UPrimitiveComponent*>(obj);

        if (!comp->GetOwner() || GPlayerLocation.GetDistanceTo(comp->K2_GetComponentLocation()) > GDistanceToDraw) {
            continue;
        }

        std::string profileName = comp->GetCollisionProfileName().ToString(); // Trigger, LineOfSight, PhysicalSurface, BlockAll, UI, PlayerPawn, OverlapAllDynamic, Custom
        //if (profileName == "PhysicalSurface" || profileName == "BlockAll") {
        //    continue;
        //}

        SDK::FVector compLocation = comp->K2_GetComponentLocation();
        SDK::FVector projected = HUD->Project(compLocation, false);

        // Only draw if it's on screen
        if (projected.Z > 0) {
            SDK::FVector2D screenPos(projected.X, projected.Y);
            float yOffset = 0.0f;

            DrawTextWithOffset(HUD, StringToWString("Profile: " + profileName).c_str(), screenPos, yOffset, GREEN);

            SDK::ECollisionChannel objectType = comp->GetCollisionObjectType();
            std::wstring objectTypeStr = CollisionChannelToString(objectType);
            DrawTextWithOffset(HUD, (L"Type: " + objectTypeStr).c_str(), screenPos, yOffset, BLUE);

            std::string ownerName = comp->GetOwner()->Name.ToString();
            DrawTextWithOffset(HUD, StringToWString("Owner: " + ownerName).c_str(), screenPos, yOffset, { 0.8f, 0.8f, 0.8f, 1.0f });

            std::wstring canStep = L"";
            switch (comp->CanCharacterStepUpOn) {
                case SDK::ECanBeCharacterBase::ECB_No: canStep = L"No"; break;
                case SDK::ECanBeCharacterBase::ECB_Yes: canStep = L"Yes"; break;
                case SDK::ECanBeCharacterBase::ECB_Owner: canStep = L"Owner"; break;
            }
            DrawTextWithOffset(HUD, (L"CanStep: " + canStep).c_str(), screenPos, yOffset, RED);
        }
    }
}

bool SafelyProjectCharacterAndDraw(SDK::AHUD* HUD, uint32_t characterIndex, const CachedCharacterDisplayData& cachedData, float columnWidth) {
    // TODO: This was janky but fixed some crashes. Need a PROPER fix.
    __try {
        SDK::UObject* obj = SDK::UObject::GObjects->GetByIndex(characterIndex);

        if (!obj || !obj->IsValid() || !obj->IsA(SDK::ANoceCharacter::StaticClass())) {
            return false; // Should be removed
        }

        auto character = static_cast<SDK::ANoceCharacter*>(obj);

        SDK::FVector projected = HUD->Project(character->K2_GetActorLocation(), false);
        if (projected.Z <= 0) {
            return false; // Behind camera
        }

        SDK::FVector2D currentScreenPos(projected.X, projected.Y);
        float currentX = currentScreenPos.X;
        float startY = currentScreenPos.Y;

        DrawTextColumn(HUD, cachedData.baseLines, currentX, startY, columnWidth);
        DrawTextColumn(HUD, cachedData.enemyLines, currentX, startY, columnWidth);
        DrawTextColumn(HUD, cachedData.emBaseLines, currentX, startY, columnWidth);
        DrawTextColumn(HUD, cachedData.componentLines, currentX, startY, columnWidth);
        return true; // Success
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        // TODO: Handle properly
        return false;
    }
}

void UpdateCharacterSelectionList() {
    std::unordered_set<uint32_t> liveCharacterIndices(GCharactersToDraw.begin(), GCharactersToDraw.end());

    // Remove from map if no longer live
    for (auto it = GCharacterSelectionMap.begin(); it != GCharacterSelectionMap.end();) {
        if (liveCharacterIndices.find(it->first) == liveCharacterIndices.end()) {
            it = GCharacterSelectionMap.erase(it);
        }
        else {
            ++it;
        }
    }

	// Add new characters that are in the live list but not yet in our map
    for (const auto& index : GCharactersToDraw) {
        if (GCharacterSelectionMap.find(index) == GCharacterSelectionMap.end()) {
            SDK::UObject* obj = SDK::UObject::GObjects->GetByIndex(index);
            if (obj && obj->IsValid()) {
                // Add new entry with default visibility (true).
                GCharacterSelectionMap[index] = { StringToWString(obj->GetName()), true };
            }
        }
    }
}

void ProcessCharacters(SDK::AHUD* HUD) {
    if (!GDrawCharacters || !HUD || !HUD->Canvas) {
        return;
    }

    // TODO: Move to Colors.hpp
    const SDK::FLinearColor TitleColor = { 1.0f, 0.8f, 0.0f, 1.0f };
    const SDK::FLinearColor HeaderColor = { 0.2f, 0.8f, 1.0f, 1.0f };
    const SDK::FLinearColor SubHeaderColor = { 0.8f, 0.6f, 1.0f, 1.0f };
    const SDK::FLinearColor BaseColor = { 0.9f, 0.9f, 0.9f, 1.0f };
    const SDK::FLinearColor EnemyColor = { 1.0f, 0.6f, 0.6f, 1.0f };
    const SDK::FLinearColor EmBaseColor = { 0.6f, 1.0f, 0.6f, 1.0f };
    const SDK::FLinearColor CompColor = { 1.0f, 0.7f, 0.4f, 1.0f };
    const SDK::FLinearColor BoolTrueColor = { 0.2f, 1.0f, 0.2f, 1.0f };

    if (++GCharInfoFrameCounter == CHAR_INFO_UPDATE_INTERVAL) {
        GCharInfoFrameCounter = 0;
        GCharacterDataCache.clear();
        GCharacterDataCache.reserve(GCharactersToDraw.size());

        for (const auto& index : GCharactersToDraw) {
            auto it = GCharacterSelectionMap.find(index);
            if (it != GCharacterSelectionMap.end() && !it->second.bIsVisible) {
                continue; // Skip this character if it's in our map and toggled off.
            }

            SDK::UObject* obj = SDK::UObject::GObjects->GetByIndex(index);
            if (!obj->IsValid() || !obj->IsA(SDK::ANoceCharacter::StaticClass()) || obj == GLP->PlayerController->Character) {
                continue;
            }
            auto character = static_cast<SDK::ANoceCharacter*>(obj);

            if (GPlayerLocation.GetDistanceTo(character->K2_GetActorLocation()) > GDistanceToDraw) {
                continue;
            }

            auto& cacheEntry = GCharacterDataCache[character->Index];
            auto AddBaseLine = [&](std::wstring l, std::wstring v, const SDK::FLinearColor& c) { cacheEntry.baseLines.push_back({ l, v, c }); };
            auto AddEnemyLine = [&](std::wstring l, std::wstring v, const SDK::FLinearColor& c) { cacheEntry.enemyLines.push_back({ l, v, c }); };
            auto AddEmBaseLine = [&](std::wstring l, std::wstring v, const SDK::FLinearColor& c) { cacheEntry.emBaseLines.push_back({ l, v, c }); };
            auto AddComponentLine = [&](std::wstring l, std::wstring v, const SDK::FLinearColor& c) { cacheEntry.componentLines.push_back({ l, v, c }); };

            // ANoceCharacter
            if (GCharDisplaySettings.bShowBaseInfo) {
                if (GCharDisplaySettings.bShowCharacterName) AddBaseLine(L"", StringToWString(character->GetName()), TitleColor);
                AddBaseLine(L"--- Base Character ---", L"", HeaderColor);
                if (GCharDisplaySettings.bShowCharacterName) AddBaseLine(L"Name: ", StringToWString(character->CharacterName.ToString()), BaseColor);
                if (GCharDisplaySettings.bShowCharacterTag) AddBaseLine(L"Tag: ", StringToWString(character->CharacterTag.TagName.ToString()), BaseColor);
                if (GCharDisplaySettings.bShowCharacterLevel) AddBaseLine(L"Level: ", std::to_wstring(character->CharacterLevel), BaseColor);
                if (GCharDisplaySettings.bShowAliveStatus) AddBaseLine(L"Alive: ", character->bAlive ? L"True" : L"False", character->bAlive ? BoolTrueColor : RED);
                if (GCharDisplaySettings.bShowDieCount) AddBaseLine(L"DieCount: ", std::to_wstring(character->DieCount), BaseColor);
                if (GCharDisplaySettings.bShowMovementMode) AddBaseLine(L"Movement: ", MovementModeToString(character->CacheMovementMode), BaseColor);
                if (GCharDisplaySettings.bShowSignificance) AddBaseLine(L"Significance: ", std::to_wstring(character->CurrentSignificance), BaseColor);
                if (GCharDisplaySettings.bShowActivatedByDistance && character->IsActivatedByDistance) AddBaseLine(L"ActivatedByDist", L"", BoolTrueColor);
                if (GCharDisplaySettings.bShowSpawner) AddBaseLine(L"Spawner: ", GetSafeName(character->MySpawner), BaseColor);
                if (GCharDisplaySettings.bShowTerritory) AddBaseLine(L"Territory: ", StringToWString(character->TerritoryName.ToString()), BaseColor);
                if (GCharDisplaySettings.bShowUnderAttackTimer) AddBaseLine(L"UnderAttackTimer: ", std::to_wstring(character->UnderAttackTimer), EnemyColor);
                if (GCharDisplaySettings.bShowGameTimeSinceSpawn) AddBaseLine(L"TimeSinceSpawn: ", std::to_wstring(character->GetSecondsScineBeginPlay()), BaseColor);
                if (GCharDisplaySettings.bShowAbilityInfo) AddBaseLine(L"Abilities/Effects: ", std::to_wstring(character->CharacterAbilities.Num()) + L" / " + std::to_wstring(character->StartupEffects.Num()), BaseColor);
            }
            const auto& lastDamage = character->LastDamagedResult;
            if (GCharDisplaySettings.bShowLastDamage && lastDamage.Instigator) {
                AddBaseLine(L"--- Last Damage ---", L"", SubHeaderColor);
                if (GCharDisplaySettings.bShowDamageInstigator) AddBaseLine(L"Instigator: ", GetSafeName(lastDamage.Instigator), BaseColor);
                if (GCharDisplaySettings.bShowDamageCombo) AddBaseLine(L"Combo: ", StringToWString(lastDamage.ComboName.ToString()), BaseColor);
                if (GCharDisplaySettings.bShowHealthDamage && lastDamage.HealthDamage > 0) AddBaseLine(L"HealthDmg: ", std::to_wstring(lastDamage.HealthDamage), EnemyColor);
                if (GCharDisplaySettings.bShowWinceDamage && lastDamage.WinceDamage > 0) AddBaseLine(L"WinceDmg: ", std::to_wstring(lastDamage.WinceDamage), YELLOW);
                if (GCharDisplaySettings.bShowWinceType) AddBaseLine(L"WinceType: ", ENoceWinceTypeToString(lastDamage.WinceType), BaseColor);
            }

            // ANoceEnemyCharacter
            if (character->IsA(SDK::ANoceEnemyCharacter::StaticClass())) {
                auto enemyChar = static_cast<SDK::ANoceEnemyCharacter*>(character);
                if (GCharDisplaySettings.bShowEnemyInfo) {
                    AddEnemyLine(L"--- Enemy ---", L"", HeaderColor);
                    if (GCharDisplaySettings.bShowIsFakeDead && enemyChar->bIsFakeDead) AddEnemyLine(L"IsFakeDead", L"", BoolTrueColor);
                    if (GCharDisplaySettings.bShowUnkillable && enemyChar->Unkillable) AddEnemyLine(L"Unkillable", L"", RED);
                    if (GCharDisplaySettings.bShowForceInSight && enemyChar->ForceEnemyInSight) AddEnemyLine(L"ForceInSight", L"", BoolTrueColor);
                    if (GCharDisplaySettings.bShowHitPerformGroup) AddEnemyLine(L"HitPerformGroup: ", StringToWString(enemyChar->HitPerformGroupName.ToString()), EnemyColor);
                    if (GCharDisplaySettings.bShowFakeDeadCount) AddEnemyLine(L"FakeDeadCount: ", std::to_wstring(enemyChar->FakeDeadCount), EnemyColor);
                    if (GCharDisplaySettings.bShowReviveHealthRatio) AddEnemyLine(L"ReviveHealth: ", std::to_wstring(enemyChar->ReviveHealthRatio), EnemyColor);
                    if (GCharDisplaySettings.bShowExtendAimDistance) AddEnemyLine(L"ExtendAimDist: ", std::to_wstring(enemyChar->ExtendAutoAimDistance), EnemyColor);
                    if (GCharDisplaySettings.bShowFakeDeadTimers) AddEnemyLine(L"FakeDeadTime: ", std::to_wstring(enemyChar->FakeDeadProgressTime), EnemyColor);
                    if (GCharDisplaySettings.bShowIsVisibleToPlayer) AddEnemyLine(L"VisibleToPlayer: ", enemyChar->IsVisibileToPlayer(false, 1.0f, 0, false, SDK::EDrawDebugTrace::None) ? L"Yes" : L"No", BaseColor);
                }

                if (GCharDisplaySettings.bShowEnemyAttributes && enemyChar->EnemyAttributeSet) {
                    auto attrs = enemyChar->EnemyAttributeSet;
                    AddEnemyLine(L"--- Attributes ---", L"", SubHeaderColor);
                    if (GCharDisplaySettings.bShowHealthAttack) AddEnemyLine(L"HealthAtk: ", AttributeToString(attrs->HealthAttack), EnemyColor);
                    if (GCharDisplaySettings.bShowWinceAttack) AddEnemyLine(L"WinceAtk: ", AttributeToString(attrs->WinceAttack), YELLOW);
                    if (GCharDisplaySettings.bShowDmgRatioHealth) AddEnemyLine(L"DmgRatioHealth: ", AttributeToString(attrs->DamageRatioHealth), EnemyColor);
                    if (GCharDisplaySettings.bShowDmgRatioWince) AddEnemyLine(L"DmgRatioWince: ", AttributeToString(attrs->DamageRatioWince), YELLOW);
                    if (GCharDisplaySettings.bShowJealous) AddEnemyLine(L"Jealous: ", AttributeToString(attrs->Jealous), PURPLE);
                    if (GCharDisplaySettings.bShowLinkAttackRatio) AddEnemyLine(L"LinkAtkRatio: ", AttributeToString(attrs->LinkAttackRatio), CYAN);
                    if (GCharDisplaySettings.bShowFakeDeadWakeupRatio) AddEnemyLine(L"FakeDeadWakeup: ", AttributeToString(attrs->FakeDeadWakeupRatio), BaseColor);
                }

                // Components (TODO)
                if (GCharDisplaySettings.bShowComponentInfo) {
                    AddComponentLine(L"--- Components ---", L"", HeaderColor);
                    if (GCharDisplaySettings.bShowAIController && enemyChar->EnemyAIController) {
                        auto ai = enemyChar->EnemyAIController;
                        AddComponentLine(L"AI Controller:", GetSafeName(ai), SubHeaderColor);
                        if (GCharDisplaySettings.bShowAIAlertType) AddComponentLine(L" AlertType: ", AIAlertTypeToString(ai->GetAlertType()), CompColor);
                        if (GCharDisplaySettings.bShowAIAlertness) AddComponentLine(L" Alertness: ", std::to_wstring(ai->GetAlertness()), CompColor);
                        if (GCharDisplaySettings.bShowAIThinkStatus) AddComponentLine(L" ThinkEnabled: ", ai->GetEnableThink() ? L"Yes" : L"No", CompColor);
                        if (GCharDisplaySettings.bShowAIHatestTarget) AddComponentLine(L" Target: ", GetSafeName(ai->GetHatestTarget()), CompColor);
                        if (GCharDisplaySettings.bShowAIPathStatus) AddComponentLine(L" PathStatus: ", std::to_wstring((int)ai->GetPathFollowingStatus()), CompColor);
                    }
                }
            }

            // Enemy base
            if (character->IsA(SDK::ABP_EmBase_NoceEnemyCharacter_C::StaticClass())) {
                auto emBaseChar = static_cast<SDK::ABP_EmBase_NoceEnemyCharacter_C*>(character);
                if (GCharDisplaySettings.bShowEmBaseInfo) {
                    AddEmBaseLine(L"--- EmBase ---", L"", HeaderColor);
                    if (GCharDisplaySettings.bShowIsStrafe && emBaseChar->IsStrafe) AddEmBaseLine(L"IsStrafe", L"", BoolTrueColor);
                    if (GCharDisplaySettings.bShowHopeNotToMove && emBaseChar->HopeNotToMove) AddEmBaseLine(L"HopeNotToMove", L"", BoolTrueColor);
                    if (GCharDisplaySettings.bShowEnableAIOnDamaged && emBaseChar->EnableAIOnDamaged) AddEmBaseLine(L"EnableAIOnDmg", L"", BoolTrueColor);
                    if (GCharDisplaySettings.bShowMovementSpeeds) {
                        AddEmBaseLine(L"MoveBase: ", std::to_wstring(emBaseChar->MovementSpeedBase), EmBaseColor);
                        AddEmBaseLine(L"MoveMod: ", std::to_wstring(emBaseChar->MovementSpeedModifier), EmBaseColor);
                        AddEmBaseLine(L"NormalSpeed: ", std::to_wstring(emBaseChar->Loco_NormalBaseSpeed), EmBaseColor);
                        AddEmBaseLine(L"StrafeSpeed: ", std::to_wstring(emBaseChar->Loco_StrafeBaseSpeed), EmBaseColor);
                    }
                    if (GCharDisplaySettings.bShowNavAgent) AddEmBaseLine(L"NavAgent: ", NavAgentToString(emBaseChar->NavAgentSetting), EmBaseColor);
                    if (GCharDisplaySettings.bShowIsOptimized && emBaseChar->IsCurrentOptimizeMode) AddEmBaseLine(L"OPTIMIZED", L"", YELLOW);
                    if (GCharDisplaySettings.bShowTurnMontages) AddEmBaseLine(L"TurnMontages: ", GetSafeName(emBaseChar->Loco_TurnR90), EmBaseColor);
                    if (GCharDisplaySettings.bShowDefaultCapsuleInfo) AddEmBaseLine(L"Capsule R/H: ", std::to_wstring(emBaseChar->DefaultCapsuleRadius) + L"/" + std::to_wstring(emBaseChar->DefaultCapsuleHalfHeight), EmBaseColor);
                }

                // Components
                if (GCharDisplaySettings.bShowComponentInfo) {
                    if (GCharDisplaySettings.bShowHitPerformComponent && emBaseChar->NoceHitPerform) {
                        AddComponentLine(L"HitPerform:", GetSafeName(emBaseChar->NoceHitPerform), SubHeaderColor);
                        if (GCharDisplaySettings.bShowHitPerformAsset) AddComponentLine(L" Asset: ", GetSafeName(emBaseChar->NoceHitPerform->HitPerformDataAsset), CompColor);
                        if (GCharDisplaySettings.bShowHitMoveInfo) AddComponentLine(L" HitMoveTime: ", std::to_wstring(emBaseChar->NoceHitPerform->HitMoveTimer), CompColor);
                    }
                    if (GCharDisplaySettings.bShowAttackInfoComponent && emBaseChar->NoceEnemyAttackInfo) {
                        AddComponentLine(L"AtkInfo:", GetSafeName(emBaseChar->NoceEnemyAttackInfo), SubHeaderColor);
                        if (GCharDisplaySettings.bShowEnemyComboAsset) AddComponentLine(L" ComboAsset: ", GetSafeName(emBaseChar->NoceEnemyAttackInfo->EnemyComboDataAsset), CompColor);
                    }
                    if (GCharDisplaySettings.bShowDamageHandleComponent && emBaseChar->NoceEnemyDamageHandle) AddComponentLine(L"DmgHandle:", GetSafeName(emBaseChar->NoceEnemyDamageHandle), SubHeaderColor);
                    if (GCharDisplaySettings.bShowBodyPartGroupComponent && emBaseChar->NoceBodyPartGroup) AddComponentLine(L"BodyParts:", GetSafeName(emBaseChar->NoceBodyPartGroup), SubHeaderColor);
                    if (GCharDisplaySettings.bShowAttackTraceComponent && emBaseChar->NoceAttackTrace) AddComponentLine(L"AtkTrace:", GetSafeName(emBaseChar->NoceAttackTrace), SubHeaderColor);
                    if (GCharDisplaySettings.bShowNavLinkInfo && emBaseChar->BPC_EmBase_NavLinkInfo) {
                        AddComponentLine(L"NavLink:", GetSafeName(emBaseChar->BPC_EmBase_NavLinkInfo), SubHeaderColor);
                        if (emBaseChar->BPC_EmBase_NavLinkInfo->InNavLink) AddComponentLine(L" InNavLink", L"", BoolTrueColor);
                    }
                }
            }
        }
    }

    if (GCharacterDataCache.empty()) {
        return;
    }

    const float columnWidth = 350.0f;

    for (const auto& [characterIndex, cachedData] : GCharacterDataCache) {
        SafelyProjectCharacterAndDraw(HUD, characterIndex, cachedData, columnWidth);
    }
}

SDK::FBatchedLine CreateLine(const SDK::FVector& start, const SDK::FVector& end, const SDK::FLinearColor& color, float thickness) {
    SDK::FBatchedLine line{};
    line.Start = start;
    line.End = end;
    line.Color = color;
    line.Thickness = thickness;
	line.RemainingLifeTime = 0.0f; // Persist for one frame (could also write to PersistentLines ig)
    line.DepthPriority = 0;
    return line;
}


void AddConvexHullLines(std::vector<SDK::FBatchedLine>& lines, const SDK::FKConvexElem& convex, const SDK::FTransform& worldTransform, const SDK::FLinearColor& color) {
    SDK::FTransform finalTransform = convex.Transform * worldTransform;

    for (size_t i = 0; i < convex.IndexData.Num(); i += 3) {
        SDK::FVector v0 = finalTransform.TransformPosition(convex.VertexData[convex.IndexData[i]]);
        SDK::FVector v1 = finalTransform.TransformPosition(convex.VertexData[convex.IndexData[i + 1]]);
        SDK::FVector v2 = finalTransform.TransformPosition(convex.VertexData[convex.IndexData[i + 2]]);

        lines.push_back(CreateLine(v0, v1, color));
        lines.push_back(CreateLine(v1, v2, color));
        lines.push_back(CreateLine(v2, v0, color));
    }
}

void AddBoxLines(std::vector<SDK::FBatchedLine>& lines, const SDK::FKBoxElem& box, const SDK::FTransform& worldTransform, const SDK::FLinearColor& color) {
    SDK::FVector Extents = { box.X / 2.0, box.Y / 2.0, box.Z / 2.0 };
    SDK::FVector Center = { box.Center.X, box.Center.Y, box.Center.Z };
    SDK::FQuat Rotation = box.Rotation.Quaternion();

    SDK::FTransform BoxLocalTransform(Rotation, Center, { 1.0, 1.0, 1.0 });
    SDK::FTransform FinalTransform = BoxLocalTransform * worldTransform;

    SDK::FVector Verts[8];
    Verts[0] = { -Extents.X, -Extents.Y, -Extents.Z };
    Verts[1] = { +Extents.X, -Extents.Y, -Extents.Z };
    Verts[2] = { +Extents.X, +Extents.Y, -Extents.Z };
    Verts[3] = { -Extents.X, +Extents.Y, -Extents.Z };
    Verts[4] = { -Extents.X, -Extents.Y, +Extents.Z };
    Verts[5] = { +Extents.X, -Extents.Y, +Extents.Z };
    Verts[6] = { +Extents.X, +Extents.Y, +Extents.Z };
    Verts[7] = { -Extents.X, +Extents.Y, +Extents.Z };

    for (int i = 0; i < 8; ++i) {
        Verts[i] = FinalTransform.TransformPosition(Verts[i]);
    }

    int Edges[12][2] = {
        {0, 1}, {1, 2}, {2, 3}, {3, 0},
        {4, 5}, {5, 6}, {6, 7}, {7, 4},
        {0, 4}, {1, 5}, {2, 6}, {3, 7}
    };

    for (int i = 0; i < 12; ++i) {
        lines.push_back(CreateLine(Verts[Edges[i][0]], Verts[Edges[i][1]], color));
    }
}


void DrawBoxAroundCollisionComponent_ToBuffer(SDK::UPrimitiveComponent* primitiveComp, const SDK::FLinearColor& BoxColor, float LineThickness, std::vector<SDK::FBatchedLine>& LineBuffer) {
    if (!primitiveComp || !primitiveComp->IsValidChecked()) {
        return;
    }

    SDK::FVector minBounds, maxBounds;

    if (!GetShapeLocalBounds(primitiveComp, minBounds, maxBounds)) {
        return;
    }

    SDK::FTransform componentToWorld = primitiveComp->K2_GetComponentToWorld();

    // TODO: Not sure how I feel about this one...
    if ((maxBounds - minBounds).SizeSquared() < 0.1f) {
        return;
    }

    SDK::FVector localCorners[8];
    localCorners[0] = minBounds;
    localCorners[1] = SDK::FVector(maxBounds.X, minBounds.Y, minBounds.Z);
    localCorners[2] = SDK::FVector(maxBounds.X, maxBounds.Y, minBounds.Z);
    localCorners[3] = SDK::FVector(minBounds.X, maxBounds.Y, minBounds.Z);

    localCorners[4] = SDK::FVector(minBounds.X, minBounds.Y, maxBounds.Z);
    localCorners[5] = SDK::FVector(maxBounds.X, minBounds.Y, maxBounds.Z);
    localCorners[6] = maxBounds;
    localCorners[7] = SDK::FVector(minBounds.X, maxBounds.Y, maxBounds.Z);

    SDK::FVector worldCorners[8];
    for (int i = 0; i < 8; ++i) {
        worldCorners[i] = componentToWorld.TransformPosition(localCorners[i]);
    }

    int edges[12][2] = {
        {0, 1}, {1, 2}, {2, 3}, {3, 0},
        {4, 5}, {5, 6}, {6, 7}, {7, 4},
        {0, 4}, {1, 5}, {2, 6}, {3, 7}
    };

    // Add lines to the buffer
    for (int i = 0; i < 12; ++i) {
        LineBuffer.push_back(CreateLine(worldCorners[edges[i][0]], worldCorners[edges[i][1]], BoxColor, LineThickness));
    }
}


SDK::UPrimitiveComponent* GetMainTriggerComponent(SDK::ABP_PlayerTrigger_Base_C* trigger) {
    if (!trigger || !trigger->IsValidChecked()) {
        return nullptr;
    }

    // Honestly not sure if this is needed/should be done...
    if (trigger->IsA(SDK::ABP_AutoSave_PlayerTrigger_C::StaticClass())) {
        return static_cast<SDK::ABP_AutoSave_PlayerTrigger_C*>(trigger)->Box;
    }

    if (trigger->IsA(SDK::ABP_AISpawnerEventTrigger_C::StaticClass())) {
        return static_cast<SDK::ABP_AISpawnerEventTrigger_C*>(trigger)->TrigBox;
    }

    if (trigger->MyTrigger && trigger->MyTrigger->IsA(SDK::UPrimitiveComponent::StaticClass())) {
        return static_cast<SDK::UPrimitiveComponent*>(trigger->MyTrigger);
    }

    if (trigger->CacheBoxComponent) {
        return trigger->CacheBoxComponent;
    }

	std::cout << "Unable to find main trigger component for trigger: " << trigger->GetName() << "\n";
	return nullptr; // TODO: This is BAD and should NOT happen. Find a fallback way to get a component.
}

template<typename T>
struct TArray_Hacker {
    T* Data;
    int32_t NumElements;
    int32_t MaxElements;
};

void ProcessAndDrawAllWorldLines() {
    if (!GWorld || !GLP || !GLP->PlayerController || !GLP->PlayerController->Character) {
        return;
    }

    const size_t Offset_BatchedLines = offsetof(SDK::ULineBatchComponent, BatchedLines);
    auto* BatchedLinesArray = (TArray_Hacker<SDK::FBatchedLine>*)((uintptr_t)GWorld->ForegroundLineBatcher + Offset_BatchedLines);
    if (BatchedLinesArray->Data == GOurLastFramesBuffer && GOurLastFramesBuffer != nullptr) {
        FMemory_Free(GOurLastFramesBuffer);
        BatchedLinesArray->Data = nullptr; BatchedLinesArray->NumElements = 0; BatchedLinesArray->MaxElements = 0;
    }
    GOurLastFramesBuffer = nullptr;

    GAllFrameLines.clear();
    GAllFrameLines.reserve(MAX_LINES_TO_DRAW_PER_FRAME); // TODO: Tweak

    SDK::FVector PlayerLocation = GLP->PlayerController->Character->K2_GetActorLocation();
    const float DRAW_DISTANCE_SQUARED = GDistanceToDraw * GDistanceToDraw;


    if (GDrawCustomTriggers) {
        for (auto index : GCustomTriggersToDraw) {
            // TODO: See comments in ProcessCustomTriggers
			auto* trigger = GetByIndexSafely<SDK::ABP_PlayerTrigger_Base_C>(index); // Should this be ANocePlayerTriggerBase? Unsure if any are missed this way
            if (!trigger || !trigger->IsValidChecked()) {
                continue;
            }

            if (PlayerLocation.GetDistanceTo(trigger->K2_GetActorLocation()) > GDistanceToDraw) {
                continue;
            }

            SDK::FLinearColor wireColor = GRAY;
            float lineThickness = 1.0f;

            bool bIsOverlapping = trigger->HasBeginOverlap;
            bool bIsConditionMet = trigger->IsMatchingCondition();
            bool bIsEnabled = trigger->IsEnabled;
            bool bIsDone = trigger->TriggerOnce && trigger->HasTriggered; // TODO: Ehhh

            if (bIsOverlapping) {
                wireColor = CYAN;
                lineThickness = 3.0f;
            }
            else if (bIsEnabled && bIsConditionMet && !bIsDone) {
                wireColor = LIME;
                lineThickness = 2.0f;
            }
            else if (!bIsEnabled) {
                wireColor = RED;
            }
            else if (!bIsConditionMet) {
                wireColor = YELLOW;
            }
            else if (bIsDone) {
                wireColor = GRAY;
            }

            // Draw to buffer
            if (SDK::UPrimitiveComponent* mainComp = GetMainTriggerComponent(trigger)) {
				// This draws 12 lines at most. We don't want to only draw some so we check if we have enough space first.
                if (GAllFrameLines.size() < MAX_LINES_TO_DRAW_PER_FRAME - 12) {
                    DrawBoxAroundCollisionComponent_ToBuffer(mainComp, wireColor, lineThickness, GAllFrameLines);
                }
            }
        }
    }

	// So this is a bit janky. By default, none of the line batchers are created. The engine still attempts to draw them though, so we can just make it.
    // This has caused quite a few crashes. I think that this is the best solution for now. The engine can clear the TArray so we make a new one every frame.
	// This isn't super optimal but honestly it's not a huge performance impact. Though this also depends on how many lines we draw.
    const size_t NumLines = GAllFrameLines.size();
    if (NumLines == 0) {
        return;
    }

    const size_t BufferSize = NumLines * sizeof(SDK::FBatchedLine);
    SDK::FBatchedLine* NewBuffer = (SDK::FBatchedLine*)FMemory_Malloc(BufferSize, alignof(SDK::FBatchedLine));
    if (!NewBuffer) {
        return;
    }

    memcpy(NewBuffer, GAllFrameLines.data(), BufferSize);

	// Hijack the engine's line buffer
    BatchedLinesArray->Data = NewBuffer;
    BatchedLinesArray->NumElements = NumLines;
    BatchedLinesArray->MaxElements = NumLines;

	// Need to do this for the engine to notice the change/properly update things
    MarkRenderStateDirty_Func(GWorld->ForegroundLineBatcher);

    // Store buffer pointer for cleanup next frame
    GOurLastFramesBuffer = NewBuffer;
}

void CheckWorldForCustomTriggers() {
	size_t currentSize = GCustomTriggersToDraw.size(); // If this is called when we already populated it, might as well reserve the space again
    GCustomTriggersToDraw.clear();
    GCustomTriggersToDraw.reserve(currentSize);

    for (int i = 0; i < SDK::UObject::GObjects->Num(); ++i) {
        auto obj = SDK::UObject::GObjects->GetByIndex(i);

        if (!obj || obj->IsDefaultObject() || !obj->IsValidChecked() || !obj->IsA(SDK::ANocePlayerTriggerBase::StaticClass())) {
            continue;
        }

        GCustomTriggersToDraw.push_back(i);
    }
}

// Process custom triggers: draw their collision and names/status on screen
void ProcessCustomTriggers(SDK::AHUD* HUD, SDK::FColor textColor) {
    if ((!GDrawCustomTriggers && !GDrawCustomTriggersNames) || !HUD || !HUD->Canvas) {
        return;
    }

    SDK::FLinearColor linearTextColor = {
        (float)textColor.R / 255.f,
        (float)textColor.G / 255.f,
        (float)textColor.B / 255.f,
        (float)textColor.A / 255.f
    };

    for (auto index : GCustomTriggersToDraw) {
        auto* trigger = GetByIndexSafely<SDK::ANocePlayerTriggerBase>(index);
        if (!trigger || !trigger->IsValidChecked()) {
            continue;
        }

        SDK::FVector actorLocation = trigger->K2_GetActorLocation();
        if (GPlayerLocation.GetDistanceTo(actorLocation) > GDistanceToDraw) {
            continue;
        }

        // TODO: Since this is used in another function, we should make a helper for it. Or cache it.
        SDK::FLinearColor wireColor = GRAY;
        float lineThickness = 1.0f;

        bool bIsOverlapping = false;
        bool bIsConditionMet = trigger->IsMatchingCondition();
        bool bIsEnabled = trigger->IsEnabled;
        bool bIsDone = false;
        bool bTriggerOnce = false;
        bool bTriggerWhenOverlapping = false;

        if (trigger->IsA(SDK::ABP_PlayerTrigger_Base_C::StaticClass())) {
            auto* triggerMore = static_cast<SDK::ABP_PlayerTrigger_Base_C*>(trigger);
            bIsOverlapping = triggerMore->HasBeginOverlap;
            bIsDone = triggerMore->TriggerOnce && triggerMore->HasTriggered;
            bTriggerOnce = triggerMore->TriggerOnce;
            bTriggerWhenOverlapping = triggerMore->TriggerWhenOverlapping;
        }

        if (bIsOverlapping) { // Overlapping, just triggered
            wireColor = CYAN;
            lineThickness = 3.0f;
        }
        else if (bIsEnabled && bIsConditionMet && !bIsDone) { // All conditions met, ready to trigger. TODO: Improve this
            wireColor = LIME;
            lineThickness = 2.0f;
        }
        else if (!bIsEnabled) { // Disabled for some reason (why?)
			wireColor = RED;
        }
        else if (!bIsConditionMet) { // Condition(s) not met
            wireColor = YELLOW;
        }
        else if (bIsDone) { // Triggered/Completed (No further interaction possible)
            wireColor = GRAY;
        }

        if (GDrawCustomTriggersNames) {
            SDK::FVector projectedLocation = HUD->Project(actorLocation, false);

            if (projectedLocation.Z <= 0) {
				continue; // Behind camera
            }

            SDK::FVector2D screenPosition(projectedLocation.X, projectedLocation.Y);
            float yOffset = 0.0f;

            std::string triggerName = trigger->GetName();
            DrawTextWithOffset(HUD, StringToWString(triggerName).c_str(), screenPosition, yOffset, WHITE);

            // TODO: Display this better
            DrawTextWithOffset(HUD, (L"STATUS: " + (std::wstring)(bIsDone ? L"DONE" : (bIsOverlapping ? L"OVERLAP" : (bIsConditionMet ? L"READY" : L"WAITING")))).c_str(), screenPosition, yOffset, wireColor);

            // TODO: Display this better
            DrawTextWithOffset(HUD, (L"Once/Overlap: " + (std::wstring)(bTriggerOnce ? L"Yes" : L"No") + L" / " + (bTriggerWhenOverlapping ? L"Yes" : L"No")).c_str(), screenPosition, yOffset, BaseColor);


            // Progress Tags
            if (trigger->ProgressCondition.GameplayTags.Num() > 0) {
                DrawTextWithOffset(HUD, L"Progress Tags:", screenPosition, yOffset, trigger->NeedExactProgress ? GREEN : RED);

                for (const auto& tag : trigger->ProgressCondition.GameplayTags) {
                    if (!tag.TagName.IsNone()) {
                        std::string oStr = tag.TagName.ToString();
                        std::wstring wStr = StringToWString(oStr);
                        SDK::FLinearColor tagColor = RED;

                        // Check if this progress tag is currently active (via FSM)
                        // TODO: Fix
                        bool bIsActiveTransition = false;
                        for (const auto& str : GPossibleTransitions) {
                            if (str == oStr) {
                                bIsActiveTransition = true;
                                break;
                            }
                        }

                        // Current state progress tag
                        // TODO: Pretty sure this is not working properly. Fix!
                        if (GCurrentGameStateName.find(wStr) != std::wstring::npos) {
                            tagColor = MAGENTA; // Matching our CURRENT state
                        }
                        else if (bIsActiveTransition) {
                            tagColor = ORANGE; // Possible NEXT state
                        }
                        else {
                            tagColor = BLUE; // Required but not current/next state
                        }

                        DrawTextWithOffset(HUD, (L" - " + wStr).c_str(), screenPosition, yOffset, tagColor);
                    }
                }
            }

            // Additional Tags
            // TODO: Improve
            if (trigger->AdditionalTags.GameplayTags.Num() > 0) {
                DrawTextWithOffset(HUD, L"Additional Tags:", screenPosition, yOffset, trigger->AllAdditionalTags ? GREEN : RED);
                for (const auto& tag : trigger->AdditionalTags.GameplayTags) {
                    if (!tag.TagName.IsNone()) {
                        std::string tagStr = " - " + tag.TagName.ToString();
                        // TODO: Make sure these truly match (maybe compare by FName and not string?)
                        bool bHasTag = false;
                        for (const auto& activeTag : GActiveGameplayTags) {
                            if (activeTag == StringToWString(tagStr)) {
                                bHasTag = true;
                                break;
                            }
                        }
                        DrawTextWithOffset(HUD, StringToWString(tagStr).c_str(), screenPosition, yOffset, bHasTag ? LIME : YELLOW);
                    }
                }
            }

            // Without Tags
            // TODO: Improve
            if (trigger->WithOutAdditionalTags.GameplayTags.Num() > 0) {
                DrawTextWithOffset(HUD, L"Without Tags:", screenPosition, yOffset, trigger->WithoutAllAdditionalTags ? GREEN : RED);
                for (const auto& tag : trigger->WithOutAdditionalTags.GameplayTags) {
                    if (!tag.TagName.IsNone()) {
                        std::string tagStr = " - " + tag.TagName.ToString();
                        DrawTextWithOffset(HUD, StringToWString(tagStr).c_str(), screenPosition, yOffset, CYAN);
                    }
                }
            }

            if (!trigger->CacheProgressTag.TagName.IsNone()) {
                std::string tagStr = "CacheProgressTag: " + trigger->CacheProgressTag.TagName.ToString();
                DrawTextWithOffset(HUD, StringToWString(tagStr).c_str(), screenPosition, yOffset, PURPLE);
            }

            // Dialog
            if (trigger->IsA(SDK::ABP_PlayDialogTrigger_C::StaticClass())) {
                auto dialogTrigger = static_cast<SDK::ABP_PlayDialogTrigger_C*>(trigger);
                std::wstring dialogName = StringToWString(dialogTrigger->DialogRow.RowName.ToString());
                DrawTextWithOffset(HUD, (L"Dialog Row: " + dialogName).c_str(), screenPosition, yOffset, LAVENDER);
                if (dialogTrigger->CheckIfDone) DrawTextWithOffset(HUD, L"CHECK IF DONE", screenPosition, yOffset, ORANGE);
                // TODO: Implement TriggerWhenDone
                if (dialogTrigger->TriggerWhenDone.Num() > 0) DrawTextWithOffset(HUD, L"-> Has TriggerWhenDone Events", screenPosition, yOffset, RED);
            }

            // Autosave
            else if (trigger->IsA(SDK::ABP_AutoSave_PlayerTrigger_C::StaticClass())) {
                auto saveTrigger = static_cast<SDK::ABP_AutoSave_PlayerTrigger_C*>(trigger);
                std::wstring locationName = StringToWString(saveTrigger->LocationName.RowName.ToString());
                DrawTextWithOffset(HUD, (L"Save Loc: " + locationName).c_str(), screenPosition, yOffset, GOLD);
                DrawTextWithOffset(HUD, (L"Cooldown: " + std::to_wstring(saveTrigger->CoolDownTime) + L"s").c_str(), screenPosition, yOffset, BaseColor);
                if (saveTrigger->World_Trigger_Setting.Num() > 0) DrawTextWithOffset(HUD, L"-> Sets World Triggers", screenPosition, yOffset, MAROON);
            }

            // Spawner
            else if (trigger->IsA(SDK::ABP_AISpawnerEventTrigger_C::StaticClass())) {
                auto spawnerTrigger = static_cast<SDK::ABP_AISpawnerEventTrigger_C*>(trigger);
                DrawTextWithOffset(HUD, (L"Spawner Targets: " + std::to_wstring(spawnerTrigger->TargetSpawner.Num())).c_str(), screenPosition, yOffset, FOREST_GREEN);
                DrawTextWithOffset(HUD, (L"Events: " + std::to_wstring(spawnerTrigger->Events.Num())).c_str(), screenPosition, yOffset, FOREST_GREEN);
            }

            // Map icon
            else if (trigger->IsA(SDK::ABP_AddRemoveMapIconTrigger_C::StaticClass())) {
                auto mapTrigger = static_cast<SDK::ABP_AddRemoveMapIconTrigger_C*>(trigger);
                std::wstring action = mapTrigger->IsAdd ? L"ADD" : L"REMOVE";
                std::wstring icon = std::to_wstring(static_cast<int>(mapTrigger->MapIcon));
                DrawTextWithOffset(HUD, (action + L" Icon: " + icon).c_str(), screenPosition, yOffset, SILVER);
                if (mapTrigger->AdditionalAddMapIcons.Num() > 0 || mapTrigger->AdditionalRemoveMapIcons.Num() > 0) {
                    DrawTextWithOffset(HUD, (L"+ Add/Remove " + std::to_wstring(mapTrigger->AdditionalAddMapIcons.Num() + mapTrigger->AdditionalRemoveMapIcons.Num()) + L" extra icons").c_str(), screenPosition, yOffset, SILVER);
                }
            }

            // TODO: Add more
        }
    }
    GShouldUpdateCustomTriggersVisibility = false; // TODO: Remove?
}


struct FLaunchCandidate {
    SDK::AActor* Actor;
    SDK::UPrimitiveComponent* Component;
    float Score;

    FLaunchCandidate(SDK::AActor* InActor, SDK::UPrimitiveComponent* InComp) : Actor(InActor), Component(InComp), Score(0.0f) {}
};

std::vector<FLaunchCandidate> g_LaunchCandidates;



void AnalyzeSceneForLaunchCandidates() {
    if (!GWorld || !GLP || !GLP->PlayerController || !GLP->PlayerController->Character) {
        return;
    }

    std::cout << "Starting analysis for launch canidates, GAME WILL FREEZE." << std::endl;
    auto startTime = std::chrono::high_resolution_clock::now();

    g_LaunchCandidates.clear();
    int processedComponents = 0;

    for (int i = 0; i < SDK::UObject::GObjects->Num(); ++i) {
        auto obj = SDK::UObject::GObjects->GetByIndex(i);

        if (!obj || obj->IsDefaultObject() || !obj->IsA(SDK::UPrimitiveComponent::StaticClass())) {
            continue;
        }
        auto PrimitiveComp = static_cast<SDK::UPrimitiveComponent*>(obj);
        if (!PrimitiveComp->WorldPrivate || !PrimitiveComp->GetOwner()) {
            continue;
        }

        // Must block/interact with player
        if (PrimitiveComp->GetCollisionEnabled() == SDK::ECollisionEnabled::NoCollision ||
            PrimitiveComp->GetCollisionResponseToChannel(SDK::ECollisionChannel::ECC_Pawn) != SDK::ECollisionResponse::ECR_Block) {
            continue;
        }

        SDK::UBodySetup* BodySetup = nullptr;
        // TODO: Should this be reverted?
        if (PrimitiveComp->IsA(SDK::UStaticMeshComponent::StaticClass())) {
            auto StaticMeshComp = static_cast<SDK::UStaticMeshComponent*>(PrimitiveComp);
            if (StaticMeshComp->StaticMesh) {
                BodySetup = StaticMeshComp->StaticMesh->BodySetup;
            }
        }
        // TODO: Add other types?

        if (!BodySetup) {
            continue;
        }

        processedComponents++;
        FLaunchCandidate candidate(PrimitiveComp->GetOwner(), PrimitiveComp);
        bool hasPotential = false;

        // Overrides might be interesting
        const auto& slopeOverride = BodySetup->WalkableSlopeOverride;
        if (slopeOverride.WalkableSlopeBehavior == SDK::EWalkableSlopeBehavior::WalkableSlope_Unwalkable) {
            candidate.Score += 50;
            hasPotential = true;
        }

        // Using TriMesh collision might be a pretty big red flag, kinda depends though
        if (BodySetup->TriMeshGeometries.Num() > 0) {
            candidate.Score += 200;
            hasPotential = true;
            // TODO: Would need to do furtner analysis to see if it's complex enough/weird geometry
        }

        // more "Simple" collision means less places for weird clipping stuff
        const auto& aggGeom = BodySetup->AggGeom;

        // Many convex hulls can be a red flag
        int numConvexHulls = aggGeom.ConvexElems.Num();
        if (numConvexHulls > 5) { // TODO: Tweak
            candidate.Score += numConvexHulls * 10;
            hasPotential = true;
        }

        // Not sure if this should be flagged or not...
        int totalSimpleShapes = numConvexHulls + aggGeom.BoxElems.Num() + aggGeom.SphereElems.Num() + aggGeom.SphylElems.Num();
        if (totalSimpleShapes > 10) {
            candidate.Score += totalSimpleShapes * 2;
            hasPotential = true;
        }

        if (hasPotential) {
            g_LaunchCandidates.push_back(candidate);
        }
    }

    // TODO: Also scan for APhysicsVolume?

    // Another thing that might be important as well is UABP_Pl_Hina_C slope fields/functions

    std::sort(g_LaunchCandidates.begin(), g_LaunchCandidates.end(), [](const FLaunchCandidate& a, const FLaunchCandidate& b) {
        return a.Score > b.Score;
        });


    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = endTime - startTime;
    std::cout << "Analysis complete in " << elapsed.count() << " seconds." << std::endl;
    std::cout << "Processed " << processedComponents << " components and found " << g_LaunchCandidates.size() << " potential launch candidates." << std::endl;

    // TODO: Display in ImGui
    std::cout << "--- Top 5 Launch Candidates ---\n";
    for (int i = 0; i < std::min((int)g_LaunchCandidates.size(), 5); ++i) {
        const auto& candidate = g_LaunchCandidates[i];
        std::cout << "  #" << i + 1 << " Actor: " << candidate.Actor->GetName() << " (Score: " << candidate.Score << ")\n";
    }
}


struct FBakedHazard {
    SDK::FVector BoundingCenter; // For fast distance culling
    float Score;
    std::vector<SDK::FBatchedLine> BakedLines;
};

// This is our global cache, populated ONCE by the baking function.
std::vector<FBakedHazard> g_BakedHazards;

void BakeTriangleLines(std::vector<SDK::FBatchedLine>& BakedLines,
    const SDK::FVector& WorldV0,
    const SDK::FVector& WorldV1,
    const SDK::FVector& WorldV2,
    const SDK::FLinearColor& Color) {

    BakedLines.push_back(CreateLine(WorldV0, WorldV1, Color));
    BakedLines.push_back(CreateLine(WorldV1, WorldV2, Color));
    BakedLines.push_back(CreateLine(WorldV2, WorldV0, Color));
}

void BakeAllLaunchHazards() {
    if (!GWorld || !GLP || !GLP->PlayerController || !GLP->PlayerController->Character) {
        return;
    }

    std::cout << "Starting one-time bake process... GAME WILL FREEZE." << std::endl;
    auto startTime = std::chrono::high_resolution_clock::now();

    g_BakedHazards.clear(); // Clear previous bake results

    for (int i = 0; i < SDK::UObject::GObjects->Num(); ++i) {
        auto obj = SDK::UObject::GObjects->GetByIndex(i);

        if (!obj || obj->IsDefaultObject() || !obj->IsA(SDK::UPrimitiveComponent::StaticClass())) {
            continue;
        }
        auto PrimitiveComp = static_cast<SDK::UPrimitiveComponent*>(obj);
        if (!PrimitiveComp->WorldPrivate || !PrimitiveComp->GetOwner()) {
            continue;
        }
        if (PrimitiveComp->GetCollisionEnabled() == SDK::ECollisionEnabled::NoCollision || PrimitiveComp->GetCollisionResponseToChannel(SDK::ECollisionChannel::ECC_Pawn) != SDK::ECollisionResponse::ECR_Block) {
            continue;
        }

        SDK::UBodySetup* BodySetup = nullptr;
        if (PrimitiveComp->IsA(SDK::UStaticMeshComponent::StaticClass())) {
            auto StaticMeshComp = static_cast<SDK::UStaticMeshComponent*>(PrimitiveComp);
            if (StaticMeshComp->StaticMesh) {
                BodySetup = StaticMeshComp->StaticMesh->BodySetup;
            }
        }
        if (!BodySetup) {
            continue;
        }

        float score = 0.0f;

        auto StaticMeshComp = static_cast<SDK::UStaticMeshComponent*>(PrimitiveComp);


        std::string meshName = StaticMeshComp->StaticMesh->GetFullName();

        // Should be skipped already by previous checks, but just in case
        if (meshName.find("_BmCameraHit_BCol") != std::string::npos) {
            continue;
        }

        if (meshName.find("_BmPawnHit_BCol") != std::string::npos) {
            score += 100.0f;
        }

        std::cout << "MeshName: " << meshName << "\n";

        // --- SCORING LOGIC (DONE ONCE) ---
        if (BodySetup->WalkableSlopeOverride.WalkableSlopeBehavior == SDK::EWalkableSlopeBehavior::WalkableSlope_Unwalkable) {
            score += 50.0f;
        }
        if (BodySetup->TriMeshGeometries.Num() > 0) {
            score += 250.0f;
        }
        int numConvexHulls = BodySetup->AggGeom.ConvexElems.Num();
        if (numConvexHulls > 5) {
            score += numConvexHulls * 10.0f;
        }

        if (score < 1.0f) {
            continue;
        }

        FBakedHazard hazard;
        hazard.Score = score;
        hazard.BoundingCenter = PrimitiveComp->K2_GetComponentLocation();

        SDK::FLinearColor drawColor;
        if (score >= 250.0f) { drawColor = RED; }
        else if (score >= 50.0f) { drawColor = ORANGE; }
        else { drawColor = YELLOW; }

        const SDK::FTransform WorldTransform = PrimitiveComp->K2_GetComponentToWorld();
        int indCount = 0;

        if (BodySetup->TriMeshGeometries.Num() > 0) {

            for (const auto& TriMeshPtr : BodySetup->TriMeshGeometries) {
                auto* TriMesh = TriMeshPtr.GetReference();
                if (!TriMesh) {
                    continue;
                }

                auto& Vertices = TriMesh->MParticles.MX;
                if (Vertices.Num() <= 0) {
                    continue;
                }

                if (TriMesh->MElements.bRequiresLargeIndices) {
                    auto& Indices = TriMesh->MElements.LargeIdxBuffer;
                    for (const auto& TriangleIndices : Indices) {
                        const SDK::FVector LocalV0 = { (double)Vertices[TriangleIndices.x].V[0], (double)Vertices[TriangleIndices.x].V[1], (double)Vertices[TriangleIndices.x].V[2] };
                        const SDK::FVector LocalV1 = { (double)Vertices[TriangleIndices.y].V[0], (double)Vertices[TriangleIndices.y].V[1], (double)Vertices[TriangleIndices.y].V[2] };
                        const SDK::FVector LocalV2 = { (double)Vertices[TriangleIndices.z].V[0], (double)Vertices[TriangleIndices.z].V[1], (double)Vertices[TriangleIndices.z].V[2] };

                        const SDK::FVector WorldV0 = WorldTransform.TransformPosition(LocalV0);
                        const SDK::FVector WorldV1 = WorldTransform.TransformPosition(LocalV1);
                        const SDK::FVector WorldV2 = WorldTransform.TransformPosition(LocalV2);

                        BakeTriangleLines(hazard.BakedLines, WorldV0, WorldV1, WorldV2, drawColor);
                    }
                    indCount += Indices.Num();
                }
                else {
                    auto& Indices = TriMesh->MElements.SmallIdxBuffer;
                    for (const auto& TriangleIndices : Indices) {
                        const SDK::FVector LocalV0 = { (double)Vertices[TriangleIndices.x].V[0], (double)Vertices[TriangleIndices.x].V[1], (double)Vertices[TriangleIndices.x].V[2] };
                        const SDK::FVector LocalV1 = { (double)Vertices[TriangleIndices.y].V[0], (double)Vertices[TriangleIndices.y].V[1], (double)Vertices[TriangleIndices.y].V[2] };
                        const SDK::FVector LocalV2 = { (double)Vertices[TriangleIndices.z].V[0], (double)Vertices[TriangleIndices.z].V[1], (double)Vertices[TriangleIndices.z].V[2] };

                        const SDK::FVector WorldV0 = WorldTransform.TransformPosition(LocalV0);
                        const SDK::FVector WorldV1 = WorldTransform.TransformPosition(LocalV1);
                        const SDK::FVector WorldV2 = WorldTransform.TransformPosition(LocalV2);

                        BakeTriangleLines(hazard.BakedLines, WorldV0, WorldV1, WorldV2, drawColor);
                    }
                    indCount += Indices.Num();
                }
            }
        }

        for (const auto& convex : BodySetup->AggGeom.ConvexElems) {
            AddConvexHullLines(hazard.BakedLines, convex, WorldTransform, drawColor);
        }
        for (const auto& box : BodySetup->AggGeom.BoxElems) {
            AddBoxLines(hazard.BakedLines, box, WorldTransform, drawColor);
        }

        auto agg = BodySetup->AggGeom;

        g_BakedHazards.push_back(hazard);

        std::cout << "Name: " << PrimitiveComp->GetFullName() << " Score: " << score << ". numInd: " << indCount << "\n";

    }

    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = endTime - startTime;
    std::cout << "[Baker] Bake complete in " << elapsed.count() << " seconds." << std::endl;
    std::cout << "[Baker] Pre-computed " << g_BakedHazards.size() << " physics hazards." << std::endl;
}


//void processDevTeleports(SDK::AHUD* HUD, SDK::FColor textColor) {
//    if (!drawTeleports && !drawTeleportsNames && !shouldUpdateCustomTriggersVisibility) {
//        return;
//    }
//
//
//    SDK::FLinearColor linearTextColor = {
//        (float)textColor.R / 255.f,
//        (float)textColor.G / 255.f,
//        (float)textColor.B / 255.f,
//        (float)textColor.A / 255.f
//    };
//
//    for (auto teleport : teleportPointsToDraw) {
//        if (teleport == nullptr) continue;
//
//        if (shouldUpdateCustomTriggersVisibility) {
//            teleport->SetActorHiddenInGame(!drawTeleports);
//            if (teleport->Cube) {
//                teleport->Cube->SetHiddenInGame(!drawTeleports, true);
//            }
//            if (teleport->TextRender) {
//                teleport->TextRender->SetHiddenInGame(!drawTeleports, true);
//            }
//            if (teleport->Arrow) {
//                teleport->Arrow->SetHiddenInGame(!drawTeleports, true);
//            }
//            if (teleport->Capsule) {
//                teleport->Capsule->SetHiddenInGame(!drawTeleports, true);
//            }
//        }
//
//        SDK::FVector actorLocation = teleport->K2_GetActorLocation();
//        double distance = PlayerLocation.GetDistanceTo(actorLocation);
//
//        if (distance > distanceToDraw) {
//            continue;
//        }
//
//        // Project the world location to screen space
//        SDK::FVector projectedLocation = HUD->Project(actorLocation, false);
//
//        if (projectedLocation.Z <= 0) { // Also skip if it's behind the camera.
//            continue;
//        }
//
//        SDK::FVector2D screenPosition(projectedLocation.X, projectedLocation.Y);
//
//        if (drawTeleports) {
//            if (teleport->Cube) {
//                DrawBoxAroundSceneComponentHUD(teleport->Cube, HUD, GREEN, 2.0f);
//            }
//            // TODO: Draw collision radius
//
//            // Draw a line representing the teleport offset
//            SDK::FVector offsetEndLocation = actorLocation + teleport->TeleportOffset;
//            SDK::FVector projectedOffsetEnd = HUD->Project(offsetEndLocation, false);
//            if (projectedOffsetEnd.Z > 0) {
//                HUD->DrawLine(screenPosition.X, screenPosition.Y, projectedOffsetEnd.X, projectedOffsetEnd.Y, RED, 2.0f);
//            }
//        }
//
//        // Draw the name text if enabled
//        if (drawTeleportsNames) {
//            float yOffset = 0.0f; // Reset Y offset for each actor
//
//            // 1. Point Name & Category
//            std::string nameStr = teleport->PointName.ToString();
//            std::string categoryStr = teleport->PointCategory.ToString();
//            DrawTextWithOffset(HUD, StringToWString("Name: " + nameStr).c_str(), screenPosition, yOffset, linearTextColor);
//            DrawTextWithOffset(HUD, StringToWString("Category: " + categoryStr).c_str(), screenPosition, yOffset, linearTextColor);
//
//            // 2. Associated Progress Tags
//            DrawTextWithOffset(HUD, L"Progress Tags:", screenPosition, yOffset, BLUE);
//            for (const auto& tag : teleport->AssociatedProgressTags) {
//                if (!tag.TagName.IsNone()) {
//                    std::string tagStr = " - " + tag.TagName.ToString();
//                    DrawTextWithOffset(HUD, StringToWString(tagStr).c_str(), screenPosition, yOffset, BLUE);
//                }
//            }
//
//            // 3. World Triggers
//            DrawTextWithOffset(HUD, L"World Triggers:", screenPosition, yOffset, RED);
//            if (teleport->WorldTriggerSetting.Num() > 0) {
//                for (const auto& triggerName : teleport->WorldTriggerSetting) {
//                    if (!triggerName.IsNone()) {
//                        std::string triggerStr = " - " + triggerName.ToString();
//                        DrawTextWithOffset(HUD, StringToWString(triggerStr).c_str(), screenPosition, yOffset, RED);
//                    }
//                }
//            }
//            else {
//                DrawTextWithOffset(HUD, L" - None", screenPosition, yOffset, RED);
//            }
//        }
//    }
//
//    if (shouldUpdateCustomTriggersVisibility) {
//        shouldUpdateCustomTriggersVisibility = false;
//    }
//}
//
//void processSpawners(SDK::AHUD* HUD) {
//    if (!drawSpawners) return;
//    for (auto spawner : spawnersToDraw) {
//        if (!spawner) continue;
//        SDK::FVector projected = HUD->Project(spawner->K2_GetActorLocation(), false);
//        if (projected.Z > 0) {
//            float yOffset = 0.0f;
//            DrawTextWithOffset(HUD, L"AI Spawner", { projected.X, projected.Y }, yOffset, RED);
//        }
//    }
//}
//
//void processInteractables(SDK::AHUD* HUD) {
//    if (!drawInteractables) return;
//    for (auto interactable : interactablesToDraw) {
//        if (!interactable) continue;
//        SDK::FVector projected = HUD->Project(interactable->K2_GetActorLocation(), false);
//        if (projected.Z > 0) {
//            float yOffset = 0.0f;
//            SDK::FText prompt = interactable->GetPromptTextBP();
//            DrawTextWithOffset(HUD, prompt.GetStringRef(), { projected.X, projected.Y }, yOffset, { 0.8f, 0.8f, 1.0f, 1.0f });
//        }
//    }
//}
//
//void processSavePoints(SDK::AHUD* HUD) {
//    if (!drawSavePoints) return;
//    for (auto savePoint : savePointsToDraw) {
//        if (!savePoint) continue;
//        SDK::FVector projected = HUD->Project(savePoint->K2_GetActorLocation(), false);
//        if (projected.Z > 0) {
//            float yOffset = 0.0f;
//            DrawTextWithOffset(HUD, L"Save Point", { projected.X, projected.Y }, yOffset, { 1.0f, 1.0f, 0.0f, 1.0f });
//        }
//    }
//}
//
//void processBillboards(SDK::AHUD* HUD, SDK::FColor textColor) {
//    if (!drawBillboardsNames && !shouldUpdateBillboardsVisibility)
//        return;
//
//    for (auto billboard : billboardsToDraw) {
//        if (billboard == nullptr) continue;
//
//        if (shouldUpdateBillboardsVisibility) {
//            billboard->SetHiddenInGame(!drawBillboards, false);
//            billboard->SetVisibility(drawBillboards, false);
//            shouldUpdateBillboardsVisibility = false;
//        }
//
//
//        if (drawBillboardsNames) {
//            std::string tempString;
//
//            if (billboard->GetOwner() != nullptr) {
//                tempString = billboard->GetOwner()->Name.ToString();
//            }
//            else {
//                tempString = billboard->Name.ToString();
//            }
//
//            std::wstring tempWString = StringToWString(tempString);
//            SDK::FString actorName = SDK::FString(tempWString.c_str());
//
//            HUD->AddDebugText(actorName, billboard->GetOwner(), 0.0f, { 0, 0, 0 }, { 0, 0, 0 }, textColor, true, billboard->bAbsoluteLocation, true, font, 1.0f, false);
//        }
//    }
//}