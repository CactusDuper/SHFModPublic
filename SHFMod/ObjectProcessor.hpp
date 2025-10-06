#pragma once

#include "SDK.hpp"
#include <numbers>


#define CC(Object, ClassType) \
Object->IsA(ClassType::StaticClass())

#define CHECK_AND_CAST(Object, ClassType) \
(Object->IsA(ClassType::StaticClass()) ? static_cast<ClassType*>(Object) : nullptr)

#define CAST_(Object, ClassType) \
static_cast<ClassType*>(Object)

#define CAC(Object, ClassType, Function) \
if (Object->IsA(ClassType::StaticClass())){ \
auto x = CAST_(Object, ClassType); \
Function(x, HUD, font); \
}

//bool showTriggers = false;
//bool showCollisionComponentTriggers = false;
//bool showShapes = false;
//bool showEnemyStuff = false;
//bool showBlocking = false;
//bool showLevel = false;
//
//bool showTriggerNames = false;
//bool showCollisionComponentTriggerNames = false;
//bool showShapeNames = false;
//bool showBlockingName = false;
//bool showLevelName = false;

//float distanceToDraw = 500.0f;





SDK::FVector ConvertQuatToEuler(const SDK::FQuat& quat) {
    SDK::FVector euler;

    // Roll (X-axis rotation)
    euler.X = atan2(2 * (quat.X * quat.Y + quat.W * quat.Z), quat.W * quat.W + quat.X * quat.X - quat.Y * quat.Y - quat.Z * quat.Z) * 180.0f / std::numbers::pi;

    // Pitch (Y-axis rotation)
    euler.Y = asin(2 * (quat.W * quat.Y - quat.X * quat.Z)) * 180.0f / std::numbers::pi;

    // Yaw (Z-axis rotation)
    euler.Z = atan2(2 * (quat.W * quat.X + quat.Y * quat.Z), quat.W * quat.W - quat.X * quat.X - quat.Y * quat.Y + quat.Z * quat.Z) * 180.0f / std::numbers::pi;

    return euler;
}

std::wstring FormatFloat(float value) {
    // Convert the float to a string with a fixed number of decimal places
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(2) << value;

    // Get the string representation of the float
    std::string strValue = ss.str();

    // Add trailing zeros to reach a total width of 8 characters
    if (strValue.length() < 8) {
        strValue.append(8 - strValue.length(), '0');
    }

    // Convert the formatted string to a wide string
    std::wstring wstrValue(strValue.begin(), strValue.end());

    return wstrValue;
}

void DrawFVectorValuesOnHUD(SDK::AHUD* HUD, SDK::FVector vector, int32_t x, int32_t y, SDK::UFont* font, SDK::FLinearColor color, float scale, const std::wstring& str = L"") {

    std::wstring formattedValue = str + L" X: " + FormatFloat(vector.X) + L"  Y: " + FormatFloat(vector.Y) + L"  Z: " + FormatFloat(vector.Z);// +str;
    HUD->DrawText(SDK::FString(formattedValue.c_str()), color, (float)x, (float)y, font, scale, false);

}

void DrawEulerAnglesOnHUD(SDK::AHUD* HUD, const SDK::FQuat& quat, int32_t x, int32_t y, SDK::UFont* font, SDK::FLinearColor color, float scale, const std::wstring& str = L"") {
    // Convert the quaternion to Euler angles
    SDK::FVector euler = ConvertQuatToEuler(quat);

    std::wstring formattedValue = str + L" R: " + FormatFloat(euler.X) + L"  P: " + FormatFloat(euler.Y);// +str;//L"  Y: " + FormatFloat(euler.Z) + str;
    HUD->DrawText(SDK::FString(formattedValue.c_str()), color, (float)x, (float)y, font, scale, false);
}

void drawPlayerMovement(SDK::UCharacterMovementComponent* playerMoveComponent, SDK::AHUD* HUD, SDK::UFont* font, float screenX, float screenY) {


    DrawFVectorValuesOnHUD(HUD, playerMoveComponent->Velocity, screenX, screenY, font, RED, 1, L"Vel: ");
    DrawFVectorValuesOnHUD(HUD, playerMoveComponent->Acceleration, screenX, screenY + 20, font, RED, 1, L"Acc: ");
    DrawFVectorValuesOnHUD(HUD, playerMoveComponent->LastUpdateLocation, screenX, screenY + 60, font, RED, 1, L"Loc: ");
    DrawEulerAnglesOnHUD(HUD, playerMoveComponent->LastUpdateRotation, screenX, screenY + 80, font, RED, 1, L"Rot: ");

}

//void DrawALOTF2CharacterInfo(ABaseAICharacter_BP_C* enemy, AHUD* HUD, UFont* font) {
//    if (globals::showEnemies && enemy) {
//        //enemy->bDebugAttributesEnabled = true;
//        // Check if the enemy is hidden (you can add more conditions if needed)
//        //if (enemyCharacter->IsHidden()) {
//        //    return;
//        //}
//
//        // Get the enemy's location
//        FVector enemyLocation = enemy->GetLastValidLocation();
//        FVector screenLocation3d = HUD->Project(enemyLocation, 0);
//        FVector2D screenLocation = { screenLocation3d.X, screenLocation3d.Y };
//
//
//        // Draw additional information
//
//        wstring enemyHealth = L"Health: " + to_wstring(enemy->HealthComponent->GetCurrentHealth());
//
//        wstring enemyInfo = enemyHealth + L"\nName: " + StringToWString(enemy->Name.ToString());
//
//        if (enemy->AggroComponent->CurrentTargetASC) {
//            enemyInfo = enemyInfo + L"\nAGGRO";
//        }
//
//        HUD->AddDebugText(enemyInfo.c_str(), enemy, 0.0f, { 0, 0, 0 }, { 0, 0, 0 }, fGREEN, 1, false, 1, font, 1, 0);
//
//    }
//}

void drawName(SDK::AActor* actor, SDK::AHUD* HUD, SDK::FColor textColor, bool absoluteLocation, SDK::UFont* font) {

    std::string tempString = actor->Name.ToString();
    std::wstring tempWString = StringToWString(tempString);
    SDK::FString actorName = SDK::FString(tempWString.c_str());
    HUD->AddDebugText(actorName, actor, 0.0f, { 0, 0, 0 }, { 0, 0, 0 }, textColor, 1, absoluteLocation, 1, font, 1, 0);
}


void drawName(SDK::UShapeComponent* shapeComponent, SDK::AHUD* HUD, SDK::FColor textColor, bool absoluteLocation, SDK::UFont* font) {

    if (shapeComponent->GetOwner() && !(shapeComponent->GetOwner()->IsA(SDK::ATriggerBase::StaticClass())) && !(shapeComponent->IsA(SDK::ATriggerBase::StaticClass()))) {
        std::string tempString = shapeComponent->GetOwner()->Name.ToString();

        std::wstring tempWString = StringToWString(tempString);
        SDK::FString actorName = SDK::FString(tempWString.c_str());


        //FLinearColor textColor = RED;
        //HUD->AddDebugText(actorName, ownerActor, 0.0f, { 0, 0, 0 }, { 0, 0, 0 }, fRED, 1, shapeComponent->bAbsoluteLocation, 1, font, 1, 0);
        //HUD->AddDebugText(shapeType, ownerActor, 0.0f, { 0, 0, 0 }, { 0, 0, 0 }, fRED, 1, absoluteLocation, 1, font, 1, 0);
        HUD->AddDebugText(actorName, shapeComponent->GetOwner(), 0.0f, { 0, 0, 0 }, { 0, 0, 0 }, textColor, 1, absoluteLocation, 1, font, 1, 0);
    }
}








void DrawVolumeWireframe(SDK::AVolume* Volume, SDK::AHUD* HUD, SDK::FLinearColor LineColor, float LineThickness)
{
    if (!Volume || !HUD)
    {
        return; // Check for valid pointers
    }

    SDK::FTransform ComponentTransform = Volume->BrushComponent->K2_GetComponentToWorld();
    SDK::FVector PlayerLocation = HUD->PlayerOwner->Character->CapsuleComponent->RelativeLocation;
    float DistanceToPlayer = PlayerLocation.Dist(ComponentTransform.Translation);

    SDK::UModel* model = Volume->Brush;

    if (DistanceToPlayer <= globals::distanceToDraw)
    {
        SDK::UBodySetup* BrushBodySetup = Volume->BrushComponent->BrushBodySetup;
        SDK::FKAggregateGeom aggGeom = BrushBodySetup->AggGeom;

        for (int i = 0; i < aggGeom.ConvexElems.Num(); i++)
        {
            const SDK::FKConvexElem ConvexElem = aggGeom.ConvexElems[i];

            for (int32_t Index = 0; Index < aggGeom.ConvexElems[i].IndexData.Num(); Index += 3)
            {
                SDK::FVector Start = ComponentTransform.TransformPosition(aggGeom.ConvexElems[i].VertexData[aggGeom.ConvexElems[i].IndexData[Index]]);
                SDK::FVector End1 = ComponentTransform.TransformPosition(aggGeom.ConvexElems[i].VertexData[aggGeom.ConvexElems[i].IndexData[Index + 1]]);
                SDK::FVector End2 = ComponentTransform.TransformPosition(aggGeom.ConvexElems[i].VertexData[aggGeom.ConvexElems[i].IndexData[Index + 2]]);

                SDK::FVector ScreenStart = HUD->Project(Start, false);
                SDK::FVector ScreenEnd1 = HUD->Project(End1, false);
                SDK::FVector ScreenEnd2 = HUD->Project(End2, false);

                if (ScreenStart.Z > 0 && ScreenEnd1.Z > 0 && ScreenEnd2.Z > 0)
                {
                    // Check if all points are in front of the camera (Z > 0)
                    HUD->DrawLine(ScreenStart.X, ScreenStart.Y, ScreenEnd1.X, ScreenEnd1.Y, LineColor, LineThickness);
                    HUD->DrawLine(ScreenEnd1.X, ScreenEnd1.Y, ScreenEnd2.X, ScreenEnd2.Y, LineColor, LineThickness);
                    HUD->DrawLine(ScreenEnd2.X, ScreenEnd2.Y, ScreenStart.X, ScreenStart.Y, LineColor, LineThickness);
                }
            }
        }

        for (int i = 0; i < aggGeom.BoxElems.Num(); i++)
        {
            const SDK::FKBoxElem BoxElem = aggGeom.BoxElems[i];

            //void DrawDebugBox(class UObject* WorldContextObject, const struct FVector& Center, const struct FVector& Extent, const struct FLinearColor& LineColor, const struct FRotator& Rotation, float Duration, float Thickness);
            //kismet->DrawDebugBox(GWorld, BoxElem.Center, BoxElem.exten)


            // Calculate half extents of the box
            SDK::FVector HalfExtents(BoxElem.X / 2, BoxElem.Y / 2, BoxElem.Z / 2);

            // Calculate the box's vertices
            SDK::FVector Vertices[8];
            Vertices[0] = SDK::FVector(-HalfExtents.X, -HalfExtents.Y, -HalfExtents.Z);
            Vertices[1] = SDK::FVector(-HalfExtents.X, -HalfExtents.Y, HalfExtents.Z);
            Vertices[2] = SDK::FVector(-HalfExtents.X, HalfExtents.Y, -HalfExtents.Z);
            Vertices[3] = SDK::FVector(-HalfExtents.X, HalfExtents.Y, HalfExtents.Z);
            Vertices[4] = SDK::FVector(HalfExtents.X, -HalfExtents.Y, -HalfExtents.Z);
            Vertices[5] = SDK::FVector(HalfExtents.X, -HalfExtents.Y, HalfExtents.Z);
            Vertices[6] = SDK::FVector(HalfExtents.X, HalfExtents.Y, -HalfExtents.Z);
            Vertices[7] = SDK::FVector(HalfExtents.X, HalfExtents.Y, HalfExtents.Z);

            for (int32_t Index = 0; Index < 8; Index++)
            {
                SDK::FVector Start = ComponentTransform.TransformPosition(Vertices[Index]);

                // Project the vertex onto the HUD
                SDK::FVector ScreenStart = HUD->Project(Start, false);

                if (ScreenStart.Z > 0)
                {
                    int32_t NextIndex = (Index + 1) % 8;
                    SDK::FVector End = ComponentTransform.TransformPosition(Vertices[NextIndex]);
                    SDK::FVector ScreenEnd = HUD->Project(End, false);

                    // Draw a line between the current vertex and the next one
                    if (ScreenEnd.Z > 0)
                    {
                        HUD->DrawLine(ScreenStart.X, ScreenStart.Y, ScreenEnd.X, ScreenEnd.Y, LineColor, LineThickness);
                    }
                }
            }

        }
    }
}


void triggerProcessor(SDK::ATriggerBase* trigger, SDK::AHUD* HUD, SDK::UFont* font) {
    if (globals::showTriggers) {
        trigger->SetActorHiddenInGame(0);
    }
    else {
        trigger->SetActorHiddenInGame(1);
    }

    //if (trigger->CollisionComponent)
    //    trigger->CollisionComponent->SetHiddenInGame(showCollisionComponentTriggers, 0);

    SDK::FVector componentLocation = trigger->K2_GetActorLocation();
    SDK::FVector screenLocation3d = HUD->Project(componentLocation, 0);
    SDK::FVector2D screenLocation = { screenLocation3d.X, screenLocation3d.Y };

    if(globals::showTriggerNames)
        drawName(trigger, HUD, fRED, 0, font);
}

void shapeProcessor(SDK::UShapeComponent* shapeComponent, SDK::AHUD* HUD, SDK::UFont* font) {

    //if ((shapeComponent->GetOwner() && !CC(shapeComponent->GetOwner(), ATriggerBase)) || CC(shapeComponent, ATriggerBase)) {
    //    return;
    //}

    if (globals::showShapes) {
        shapeComponent->SetHiddenInGame(0, 0);
    }
    else {
        shapeComponent->SetHiddenInGame(1, 0);
    }

    float offset = 10.0f;
    float scaleFactor = 0.1f;

    AActor* ownerActor = shapeComponent->GetOwner();

    if (ownerActor && !CC(ownerActor, ATriggerBase) && !CC(shapeComponent, ATriggerBase)) {

        if (CC(shapeComponent, UBoxComponent)) {
            auto x = CAST_(shapeComponent, UBoxComponent);
            offset = x->GetScaledBoxExtent().Z * scaleFactor;
        }
        else if (CC(shapeComponent, USphereComponent)) {
            auto x = CAST_(shapeComponent, USphereComponent);
            offset = x->GetScaledSphereRadius() * scaleFactor;
        }
        else if (CC(shapeComponent, UCapsuleComponent)) {
            auto x = CAST_(shapeComponent, UCapsuleComponent);
            offset = x->GetScaledCapsuleRadius() * scaleFactor;
        }
    }

    FVector componentLocation = shapeComponent->RelativeLocation;
    FVector screenLocation3d = HUD->Project(componentLocation, 0);
    FVector2D screenLocation = { screenLocation3d.X, screenLocation3d.Y };

    if (globals::showShapeNames) {
        drawName(shapeComponent, HUD, fRED, shapeComponent->bAbsoluteLocation, font);
    }
}

void blockingVolumeProcessor(ABlockingVolume* blockingVolume, AHUD* HUD, UFont* font) {
    if (globals::showBlockingVolumes) {
        FTransform ComponentTransform = blockingVolume->BrushComponent->K2_GetComponentToWorld();
        FVector PlayerLocation = HUD->PlayerOwner->Character->CapsuleComponent->RelativeLocation;
        float DistanceToPlayer = PlayerLocation.Dist(ComponentTransform.Translation);

        if (DistanceToPlayer <= globals::distanceToDraw)
            DrawVolumeWireframe(blockingVolume, HUD, BLUE, 1.0f);
    }

    if (globals::showBlockingVolumeNames) {
        FTransform ComponentTransform = blockingVolume->BrushComponent->K2_GetComponentToWorld();
        FVector PlayerLocation = HUD->PlayerOwner->Character->CapsuleComponent->RelativeLocation;
        float DistanceToPlayer = PlayerLocation.Dist(ComponentTransform.Translation);

        if (DistanceToPlayer <= globals::distanceToDraw)
            drawName(blockingVolume, HUD, fRED, 0, font);
    }
}

void levelStreamingVolumeProcessor(ALevelStreamingVolume* levelVolume, AHUD* HUD, UFont* font) {
    if (globals::showLevelStreamingVolumes) {
        FTransform ComponentTransform = levelVolume->BrushComponent->K2_GetComponentToWorld();
        FVector PlayerLocation = HUD->PlayerOwner->Character->CapsuleComponent->RelativeLocation;
        float DistanceToPlayer = PlayerLocation.Dist(ComponentTransform.Translation);

        if (DistanceToPlayer <= globals::distanceToDraw)
            DrawVolumeWireframe(levelVolume, HUD, BLUE, 1.0f);
    }

    if (globals::showLevelStreamingVolumeNames) {
        FTransform ComponentTransform = levelVolume->BrushComponent->K2_GetComponentToWorld();
        FVector PlayerLocation = HUD->PlayerOwner->Character->CapsuleComponent->RelativeLocation;
        float DistanceToPlayer = PlayerLocation.Dist(ComponentTransform.Translation);

        if (DistanceToPlayer <= globals::distanceToDraw)
            drawName(levelVolume, HUD, fRED, 0, font);
    }
}

void objectProcessor(AHUD* HUD) {

    UFont* font = GetFont();

    //keycheck();

    for (int i = 0; i < UObject::GObjects->Num(); i++) {
        UObject* Obj = UObject::GObjects->GetByIndex(i);

        if ((Obj && !Obj->IsDefaultObject())) {

            CAC(Obj, ATriggerBase, triggerProcessor)
            else CAC(Obj, UShapeComponent, shapeProcessor)
            else CAC(Obj, ABlockingVolume, blockingVolumeProcessor)
            else CAC(Obj, ALevelStreamingVolume, levelStreamingVolumeProcessor)
            else CAC(Obj, ABaseAICharacter_BP_C, DrawALOTF2CharacterInfo)
        }
            
            //else if (CC(Obj, UDeveloperMenuSubsystem)) {
        //    if (!developerMenuSubsystem)
        //        developerMenuSubsystem = CAST_(Obj, UDeveloperMenuSubsystem);
        //}
    }

    UGameplayStatics* statics = UObject::FindObject<UGameplayStatics>("GameplayStatics Engine.Default__GameplayStatics");
    UKismetSystemLibrary* kismet = UObject::FindObject<UKismetSystemLibrary>("KismetSystemLibrary Engine.Default__KismetSystemLibrary");
    



    //if (developerMenuSubsystem && gameplayStatics) {
    //    cout << "GAMING" << endl;
    //    if (!developerMenuSubsystem->DevSettings) {
    //        cout << "SOMETHING" << endl;
    //        TSubclassOf<UObject> ObjectClass = UDeveloperMenuSettingsRuntime::StaticClass();
    //        UObject* Outer = developerMenuSubsystem;
    //        developerMenuSubsystem->DevSettings = static_cast<UDeveloperMenuSettingsRuntime*>(gameplayStatics->SpawnObject(ObjectClass, Outer));
    //    }
    //    else
    //        cout << "DevSettings is: " << developerMenuSubsystem->DevSettings << endl;
    //}

    //, UGameplayStatics* gameplayStatics

    
}





void processPlayer(AAnathemaPlayerCharacter_BP_C* AnathemaPlayer, UQuestDirector* QuestDirector, AHUD* HUD, UFont* font, float screenX, float screenY) {
    ALOTF2PlayerState_BP_C* AnathemaPlayerState = static_cast<ALOTF2PlayerState_BP_C*>(AnathemaPlayer->PlayerState);

    if (AnathemaPlayer != nullptr) {
        UCharacterMovementComponent* playerMoveComponent = AnathemaPlayer->CharacterMovement;
        UCapsuleComponent* playerCollisionComponent = AnathemaPlayer->CapsuleComponent;
        if (playerMoveComponent && playerCollisionComponent) {
            UValidLocationTrackingComponent* ValidLocationTrackingComponent = AnathemaPlayer->ValidLocationTrackingComponent;
            drawPlayerMovement(playerMoveComponent, HUD, font, screenX, screenY);
            drawPlayerValidLocation(ValidLocationTrackingComponent, HUD, font, screenX, screenY);
        }
    }
}


//
//
//
//void DrawBox(FPrimitiveDrawInterface* PDI, const FMatrix& BoxToWorld, const FVector& Radii, const FMaterialRenderProxy* MaterialRenderProxy, uint8 DepthPriorityGroup)
//{
//    // Calculate verts for a face pointing down Z
//    FVector3f Positions[4] =
//    {
//        FVector3f(-1, -1, +1),
//        FVector3f(-1, +1, +1),
//        FVector3f(+1, +1, +1),
//        FVector3f(+1, -1, +1)
//    };
//    FVector2f UVs[4] =
//    {
//        FVector2f(0,0),
//        FVector2f(0,1),
//        FVector2f(1,1),
//        FVector2f(1,0),
//    };
//
//    // Then rotate this face 6 times
//    FRotator3f FaceRotations[6];
//    FaceRotations[0] = FRotator3f(0, 0, 0);
//    FaceRotations[1] = FRotator3f(90.f, 0, 0);
//    FaceRotations[2] = FRotator3f(-90.f, 0, 0);
//    FaceRotations[3] = FRotator3f(0, 0, 90.f);
//    FaceRotations[4] = FRotator3f(0, 0, -90.f);
//    FaceRotations[5] = FRotator3f(180.f, 0, 0);
//
//    FDynamicMeshBuilder MeshBuilder(PDI->View->FeatureLevel);
//
//    for (int32 f = 0; f < 6; f++)
//    {
//        FMatrix44f FaceTransform = FRotationMatrix44f(FaceRotations[f]);
//
//        int32 VertexIndices[4];
//        for (int32 VertexIndex = 0; VertexIndex < 4; VertexIndex++)
//        {
//            VertexIndices[VertexIndex] = MeshBuilder.AddVertex(
//                FaceTransform.TransformPosition(Positions[VertexIndex]),
//                UVs[VertexIndex],
//                FaceTransform.TransformVector(FVector3f(1, 0, 0)),
//                FaceTransform.TransformVector(FVector3f(0, 1, 0)),
//                FaceTransform.TransformVector(FVector3f(0, 0, 1)),
//                FColor::White
//            );
//        }
//
//        MeshBuilder.AddTriangle(VertexIndices[0], VertexIndices[1], VertexIndices[2]);
//        MeshBuilder.AddTriangle(VertexIndices[0], VertexIndices[2], VertexIndices[3]);
//    }
//
//    MeshBuilder.Draw(PDI, FScaleMatrix(Radii) * BoxToWorld, MaterialRenderProxy, DepthPriorityGroup, 0.f);
//}