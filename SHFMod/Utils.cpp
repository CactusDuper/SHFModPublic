#include "Utils.hpp"

#include "SDK/Basic.hpp"
#include "SDK/CoreUObject_classes.hpp"
#include "SDK/CoreUObject_structs.hpp"
#include "SDK/Engine_structs.hpp"
#include "SDK/GameNoce_structs.hpp"
#include "SDK/GameplayAbilities_structs.hpp"
#include "SDK/Enum_EmBase_NavAgent_structs.hpp"
#include <Windows.h>
#include <cstdint>
#include <cstdio>
#include <string>
#include <stringapiset.h>


std::wstring VectorToWString(const SDK::FVector& vec) {
    wchar_t buffer[100];
    swprintf(buffer, 100, L"[X: %.1f, Y: %.1f, Z: %.1f]", vec.X, vec.Y, vec.Z);
    return buffer;
}

std::wstring RotatorToWString(const SDK::FRotator& rot) {
    wchar_t buffer[100];
    swprintf(buffer, 100, L"[P: %.1f, Y: %.1f, R: %.1f]", rot.Pitch, rot.Yaw, rot.Roll);
    return buffer;
}

std::wstring QuatToWString(const SDK::FQuat& q) {
    wchar_t buffer[200];
    swprintf(buffer, 200, L"[X: %.2f, Y: %.2f, Z: %.2f, W: %.2f]", q.X, q.Y, q.Z, q.W);
    return buffer;
}

std::wstring TransformToWString(const SDK::FTransform& transform) {
    std::wstring loc = VectorToWString(transform.Translation);
    std::wstring rot = RotatorToWString(transform.Rotation.ToRotator());
    std::wstring scale = VectorToWString(transform.Scale3D);
    std::wstring quat_raw = QuatToWString(transform.Rotation);

    return L"Loc:" + loc + L" Rot:" + rot + L" Scale:" + scale + L" | Quat:" + quat_raw;
}

std::wstring GetSafeName(SDK::UObject* obj) {
    if (obj && obj->IsValid()) {
        return StringToWString(obj->GetName());
    }
    return L"nullptr";
}

std::wstring DataTableRowToWString(const SDK::FDataTableRowHandle& handle) {
    return StringToWString(handle.RowName.ToString());
}

std::wstring CollisionChannelToString(SDK::ECollisionChannel Channel) {
    switch (Channel) {
        case SDK::ECollisionChannel::ECC_WorldStatic: return L"WorldStatic";
        case SDK::ECollisionChannel::ECC_WorldDynamic: return L"WorldDynamic";
        case SDK::ECollisionChannel::ECC_Pawn: return L"Pawn";
        case SDK::ECollisionChannel::ECC_Visibility: return L"Visibility";
        case SDK::ECollisionChannel::ECC_Camera: return L"Camera";
        case SDK::ECollisionChannel::ECC_PhysicsBody: return L"PhysicsBody";
        case SDK::ECollisionChannel::ECC_Vehicle: return L"Vehicle";
        case SDK::ECollisionChannel::ECC_Destructible: return L"Destructible";
        case SDK::ECollisionChannel::ECC_EngineTraceChannel1: return L"EngineTraceChannel1";
        case SDK::ECollisionChannel::ECC_EngineTraceChannel2: return L"EngineTraceChannel2";
        case SDK::ECollisionChannel::ECC_EngineTraceChannel3: return L"EngineTraceChannel3";
        case SDK::ECollisionChannel::ECC_EngineTraceChannel4: return L"EngineTraceChannel4";
        case SDK::ECollisionChannel::ECC_EngineTraceChannel5: return L"EngineTraceChannel5";
        case SDK::ECollisionChannel::ECC_EngineTraceChannel6: return L"EngineTraceChannel6";
        case SDK::ECollisionChannel::ECC_GameTraceChannel1: return L"GameTraceChannel1";
        case SDK::ECollisionChannel::ECC_GameTraceChannel2: return L"GameTraceChannel2";
        case SDK::ECollisionChannel::ECC_GameTraceChannel3: return L"GameTraceChannel3";
        case SDK::ECollisionChannel::ECC_GameTraceChannel4: return L"GameTraceChannel4";
        case SDK::ECollisionChannel::ECC_GameTraceChannel5: return L"GameTraceChannel5";
        case SDK::ECollisionChannel::ECC_GameTraceChannel6: return L"GameTraceChannel6";
        case SDK::ECollisionChannel::ECC_GameTraceChannel7: return L"GameTraceChannel7";
        case SDK::ECollisionChannel::ECC_GameTraceChannel8: return L"GameTraceChannel8";
        case SDK::ECollisionChannel::ECC_GameTraceChannel9: return L"GameTraceChannel9";
        case SDK::ECollisionChannel::ECC_GameTraceChannel10: return L"GameTraceChannel10";
        case SDK::ECollisionChannel::ECC_GameTraceChannel11: return L"GameTraceChannel11";
        case SDK::ECollisionChannel::ECC_GameTraceChannel12: return L"GameTraceChannel12";
        case SDK::ECollisionChannel::ECC_GameTraceChannel13: return L"GameTraceChannel13";
        case SDK::ECollisionChannel::ECC_GameTraceChannel14: return L"GameTraceChannel14";
        case SDK::ECollisionChannel::ECC_GameTraceChannel15: return L"GameTraceChannel15";
        case SDK::ECollisionChannel::ECC_GameTraceChannel16: return L"GameTraceChannel16";
        case SDK::ECollisionChannel::ECC_GameTraceChannel17: return L"GameTraceChannel17";
        case SDK::ECollisionChannel::ECC_GameTraceChannel18: return L"GameTraceChannel18";
        case SDK::ECollisionChannel::ECC_OverlapAll_Deprecated: return L"OverlapAll_Deprecated";
        case SDK::ECollisionChannel::ECC_MAX: return L"MAX";
        default: return L"Unknown/Custom";
    }
}

std::wstring MovementModeToString(SDK::EMovementMode mode) {
    switch (mode) {
        case SDK::EMovementMode::MOVE_None: return L"None";
        case SDK::EMovementMode::MOVE_Walking: return L"Walking";
        case SDK::EMovementMode::MOVE_NavWalking: return L"NavWalking";
        case SDK::EMovementMode::MOVE_Falling: return L"Falling";
        case SDK::EMovementMode::MOVE_Swimming: return L"Swimming";
        case SDK::EMovementMode::MOVE_Flying: return L"Flying";
        case SDK::EMovementMode::MOVE_Custom: return L"Custom";
        case SDK::EMovementMode::MOVE_MAX: return L"MAX";
        default: return L"Unknown";
    }
}

std::wstring ENoceBodyPartToString(SDK::ENoceBodyPart value) {
    switch (value) {
        case SDK::ENoceBodyPart::None: return L"None";
        case SDK::ENoceBodyPart::Head: return L"Head";
        case SDK::ENoceBodyPart::Chest: return L"Chest";
        case SDK::ENoceBodyPart::Stomach: return L"Stomach";
        case SDK::ENoceBodyPart::Shoulder: return L"Shoulder";
        case SDK::ENoceBodyPart::UpperArm: return L"UpperArm";
        case SDK::ENoceBodyPart::Forearm: return L"Forearm";
        case SDK::ENoceBodyPart::Hand: return L"Hand";
        case SDK::ENoceBodyPart::Thigh: return L"Thigh";
        case SDK::ENoceBodyPart::Shin: return L"Shin";
        case SDK::ENoceBodyPart::Foot: return L"Foot";
        case SDK::ENoceBodyPart::UpperBody: return L"UpperBody";
        case SDK::ENoceBodyPart::LowerBody: return L"LowerBody";
        case SDK::ENoceBodyPart::Unique00: return L"Unique00";
        case SDK::ENoceBodyPart::Unique01: return L"Unique01";
        default: return L"Unknown (" + std::to_wstring(static_cast<uint8_t>(value)) + L")";
    }
}

std::wstring ENoceWinceTypeToString(SDK::ENoceWinceType value) {
    switch (value) {
        case SDK::ENoceWinceType::None: return L"None";
        case SDK::ENoceWinceType::Addtive: return L"Additive";
        case SDK::ENoceWinceType::Stun: return L"Stun";
        case SDK::ENoceWinceType::KnockBack: return L"KnockBack";
        case SDK::ENoceWinceType::KnockDown: return L"KnockDown";
        case SDK::ENoceWinceType::HitFly: return L"HitFly";
        case SDK::ENoceWinceType::Roar: return L"Roar";
        case SDK::ENoceWinceType::Corruption: return L"Corruption";
        case SDK::ENoceWinceType::KatanaAutoDefense: return L"KatanaAutoDefense";
        case SDK::ENoceWinceType::Blast: return L"Blast";
        case SDK::ENoceWinceType::Mucus: return L"Mucus";
        case SDK::ENoceWinceType::HellModeFullBody: return L"HellModeFullBody";
        case SDK::ENoceWinceType::EventHeadache: return L"EventHeadache";
        case SDK::ENoceWinceType::EventHeadacheNoMove: return L"EventHeadacheNoMove";
        case SDK::ENoceWinceType::EventHeadacheSprint: return L"EventHeadacheSprint";
        case SDK::ENoceWinceType::EventHeadacheSlowRaiseHand: return L"EventHeadacheSlowRaiseHand";
        default: return L"Unknown (" + std::to_wstring(static_cast<uint8_t>(value)) + L")";
    }
}


std::wstring ENoceWeaponMotionTypeToString(SDK::ENoceWeaponMotionType value) {
    switch (value) {
        case SDK::ENoceWeaponMotionType::None: return L"None";
        case SDK::ENoceWeaponMotionType::Pipe: return L"Pipe";
        case SDK::ENoceWeaponMotionType::Dagger: return L"Dagger";
        case SDK::ENoceWeaponMotionType::Axe: return L"Axe";
        case SDK::ENoceWeaponMotionType::Claw: return L"Claw";
        case SDK::ENoceWeaponMotionType::ClawG: return L"ClawG";
        case SDK::ENoceWeaponMotionType::Lantern: return L"Lantern";
        case SDK::ENoceWeaponMotionType::Naginata: return L"Naginata";
        case SDK::ENoceWeaponMotionType::Katana: return L"Katana";
        default: return L"Unknown (" + std::to_wstring(static_cast<uint8_t>(value)) + L")";
    }
}

std::wstring ENoceDeathTypeToString(SDK::ENoceDeathType value) {
    switch (value) {
        case SDK::ENoceDeathType::Common: return L"Common";
        case SDK::ENoceDeathType::Neck: return L"Neck";
        case SDK::ENoceDeathType::Stomach: return L"Stomach";
        case SDK::ENoceDeathType::HitFly: return L"HitFly";
        case SDK::ENoceDeathType::Ladder: return L"Ladder";
        case SDK::ENoceDeathType::Narrow: return L"Narrow";
        case SDK::ENoceDeathType::Roar: return L"Roar";
        case SDK::ENoceDeathType::Mucus: return L"Mucus";
        case SDK::ENoceDeathType::Flame: return L"Flame";
        case SDK::ENoceDeathType::Headache: return L"Headache";
        case SDK::ENoceDeathType::HardHeadache: return L"HardHeadache";
        default: return L"Unknown (" + std::to_wstring(static_cast<uint8_t>(value)) + L")";
    }
}

std::wstring AttributeToString(const SDK::FGameplayAttributeData& Attr) {
    wchar_t buffer[100];
    swprintf(buffer, 100, L"%.1f / %.1f", Attr.CurrentValue, Attr.BaseValue);
    return buffer;
}

std::wstring NavAgentToString(SDK::Enum_EmBase_NavAgent NavAgent) {
    switch (NavAgent) {
        case SDK::Enum_EmBase_NavAgent::NewEnumerator0: return L"Unknown0";
        case SDK::Enum_EmBase_NavAgent::NewEnumerator1: return L"Unknown1";
        default: return L"NavUnknown";
    }
}

std::wstring AIAlertTypeToString(SDK::ENoceAIAlertType AlertType) {
    switch (AlertType) {
        case SDK::ENoceAIAlertType::None: return L"None";
        case SDK::ENoceAIAlertType::Idle: return L"Idle";
        case SDK::ENoceAIAlertType::Aware: return L"Aware";
        case SDK::ENoceAIAlertType::Engage: return L"Engage";
        default: return L"Unknown";
    }
}