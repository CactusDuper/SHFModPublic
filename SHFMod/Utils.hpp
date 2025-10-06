#pragma once

#include "SDK/Basic.hpp"
#include "SDK/CoreUObject_classes.hpp"
#include "SDK/CoreUObject_structs.hpp"
#include "SDK/Engine_structs.hpp"
#include "SDK/Enum_EmBase_NavAgent_structs.hpp"
#include "SDK/GameNoce_structs.hpp"
#include "SDK/GameplayAbilities_structs.hpp"
#include <Windows.h>
#include <string>
#include <stringapiset.h>

static std::wstring StringToWString(const std::string& tempString) {
    int bufferSize = MultiByteToWideChar(CP_UTF8, 0, tempString.c_str(), -1, NULL, 0);
    wchar_t* wstr = new wchar_t[bufferSize];
    MultiByteToWideChar(CP_UTF8, 0, tempString.c_str(), -1, wstr, bufferSize);
    std::wstring wstringResult(wstr);
    delete[] wstr;
    return wstringResult;
}

static std::wstring VectorToWString(const SDK::FVector& vec);

static std::wstring RotatorToWString(const SDK::FRotator& rot);

static std::wstring QuatToWString(const SDK::FQuat& q);

static std::wstring TransformToWString(const SDK::FTransform& transform);

static std::wstring GetSafeName(SDK::UObject* obj);

static std::wstring DataTableRowToWString(const SDK::FDataTableRowHandle& handle);

static std::wstring CollisionChannelToString(SDK::ECollisionChannel Channel);

static std::wstring MovementModeToString(SDK::EMovementMode mode);

static std::wstring ENoceBodyPartToString(SDK::ENoceBodyPart value);

static std::wstring ENoceWinceTypeToString(SDK::ENoceWinceType value);

static std::wstring ENoceWeaponMotionTypeToString(SDK::ENoceWeaponMotionType value);

static std::wstring ENoceDeathTypeToString(SDK::ENoceDeathType value);

static std::wstring AttributeToString(const SDK::FGameplayAttributeData& Attr);

static std::wstring NavAgentToString(SDK::Enum_EmBase_NavAgent NavAgent);

static std::wstring AIAlertTypeToString(SDK::ENoceAIAlertType AlertType);

template<typename T>
T* GetByIndexSafely(uint32_t index) {
    SDK::UObject* obj = SDK::UObject::GObjects->GetByIndex(index);
    if (obj->IsValid() && obj->IsA(T::StaticClass())) {
        return static_cast<T*>(obj);;
    }
    return nullptr;
}

template<typename T>
void CleanInvalidObjectIndices(std::vector<uint32_t>& object_indices) {
    std::erase_if(object_indices, [](uint32_t index) {
        auto* object_ptr = GetByIndexSafely<T>(index);
        return object_ptr == nullptr || !object_ptr->IsValidChecked();
        });
}