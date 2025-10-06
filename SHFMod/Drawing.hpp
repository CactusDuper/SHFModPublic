#pragma once

#include "Globals.hpp"
#include "SDK/BP_PlayerTrigger_Base_classes.hpp"
#include "SDK/Engine_classes.hpp"
#include "SDK/SMSystem_structs.hpp"
#include <string>


void DrawTextWithOffset(SDK::AHUD* HUD, const SDK::FString& Text, SDK::FVector2D BasePosition, float& YOffset, SDK::FLinearColor Color);
void DrawTextColumn(SDK::AHUD* HUD, const std::vector<DisplayLine>& lines, float& currentX, float startY, float columnWidth);
void DrawBoxAroundSceneComponentHUD(SDK::UStaticMeshComponent* staticMesh, SDK::AHUD* HUD, SDK::FLinearColor BoxColor, float LineThickness);
bool GetShapeLocalBounds(SDK::UPrimitiveComponent* primitiveComp, SDK::FVector& outMinBounds, SDK::FVector& outMaxBounds);
void DrawPlayerStatus(SDK::AHUD* HUD);
void DrawFSMHierarchyRecursive(SDK::AHUD* HUD, SDK::FSMStateMachine* StateMachine, float& X, float& Y, std::wstring Indent);
void DrawFSMInfo(SDK::AHUD* HUD);
void UpdateCollisionView();
void UpdateAllCollisionComponentsList();
void ProcessCollisions(SDK::AHUD* HUD);
bool SafelyProjectCharacterAndDraw(SDK::AHUD* HUD, uint32_t characterIndex, const CachedCharacterDisplayData& cachedData, float columnWidth);
void UpdateCharacterSelectionList();
void ProcessCharacters(SDK::AHUD* HUD);
SDK::FBatchedLine CreateLine(const SDK::FVector& start, const SDK::FVector& end, const SDK::FLinearColor& color, float thickness = 2.0f);
void AddConvexHullLines(std::vector<SDK::FBatchedLine>& lines, const SDK::FKConvexElem& convex, const SDK::FTransform& worldTransform, const SDK::FLinearColor& color);
void AddBoxLines(std::vector<SDK::FBatchedLine>& lines, const SDK::FKBoxElem& box, const SDK::FTransform& worldTransform, const SDK::FLinearColor& color);
void DrawBoxAroundCollisionComponent_ToBuffer(SDK::UPrimitiveComponent* primitiveComp, const SDK::FLinearColor& BoxColor, float LineThickness, std::vector<SDK::FBatchedLine>& LineBuffer);
SDK::UPrimitiveComponent* GetMainTriggerComponent(SDK::ABP_PlayerTrigger_Base_C* trigger);
void ProcessAndDrawAllWorldLines();
void CheckWorldForCustomTriggers();
void ProcessCustomTriggers(SDK::AHUD* HUD, SDK::FColor textColor);
void CheckWorldForShapes();
void ProcessShapes(SDK::AHUD* HUD, SDK::FColor textColor);