#include "Globals.hpp"

HWND GMainWindow = nullptr;
int GOpenGUIKey = VK_DELETE;

bool GIsGUIOpen = false;

void* GOurLastFramesBuffer = nullptr;

SDK::UWorld* GWorld = nullptr;
SDK::UGameEngine** GEngine = nullptr;
SDK::ULocalPlayer* GLP = nullptr;
SDK::USMInstance* GProgressFSM = nullptr;
SDK::UFont* GFont = nullptr;
SDK::UNoceGameProgressComponent* GProgressComponent = nullptr;
SDK::ABP_Pl_Hina_NocePlayerState_C* GPlayerState = nullptr;


PostRender_H HUDPostRenderOriginal = nullptr;
ProcessEvent OProcessEvent = nullptr;

Malloc_t FMemory_Malloc = nullptr;
Free_t FMemory_Free = nullptr;
MarkRenderStateDirty_t MarkRenderStateDirty_Func = nullptr;

bool GIsHUDReady = false;
bool GIsInGame = false;

FCharacterDisplaySettings GCharDisplaySettings;

std::unordered_map<uint32_t, FCharacterSelectionInfo> GCharacterSelectionMap;

bool GDrawShapes = false;
bool GDrawShapesNames = false;
bool GDrawCustomTriggers = false;
bool GDrawCustomTriggersNames = false;
bool GDrawAllCollisions = false;
bool GDrawCollisionNames = false;
bool GCollisionViewActive = false;
bool GDrawCharacters = false;
bool GDrawCharacterNames = false;
bool GDrawFSMInfo = false;
bool GDrawPlayerStatus = false;
bool GDrawProgressAndPlayerStateExtra = false;

bool GShouldUpdateAllCollisions = false;
bool GShouldUpdateCollisionView = false;
bool GShouldUpdateCustomTriggersVisibility = false;
bool GShouldUpdateShapesVisibility = false;
// TODO: ADD OTHERS


float GDistanceToDraw = 1000.0f;
SDK::FVector GPlayerLocation;


std::vector<uint32_t> GCharactersToDraw;
std::vector<uint32_t> GCustomTriggersToDraw;
std::vector<uint32_t> GCollisionComponentsToDraw;
std::vector<uint32_t> GShapesToDraw;
std::vector<uint32_t> GBillboardsToDraw;
std::vector<uint32_t> GTeleportPointsToDraw;
std::vector<uint32_t> GTriggersToDraw;
std::vector<uint32_t> GSpawnersToDraw;
std::vector<uint32_t> GInteractablesToDraw;
std::vector<uint32_t> GSavePointsToDraw;


std::wstring GCurrentGameStateName = L"State: Unknown";
std::vector<std::wstring> GActiveGameplayTags;
std::vector<std::string> GPossibleTransitions;

std::unordered_map<uint32_t, FOriginalComponentState> GHiddenComponentsForCollisionView;
std::unordered_map<uint32_t, FOriginalComponentState> GOriginalCollisionStates;
std::unordered_map<uint32_t, FOriginalComponentState> GOriginalShapeVisibility;
std::unordered_map<uint32_t, FOriginalComponentState> GOriginalBillboardVisibility;
std::unordered_map<uint32_t, FOriginalActorState> GOriginalTeleportVisibility;

std::vector<SDK::FBatchedLine> GAllFrameLines;

int GCharInfoFrameCounter = 0;
std::unordered_map<uint32_t, CachedCharacterDisplayData> GCharacterDataCache;