#include "stdafx.h"
#include "Globals.hpp"
#include "Drawing.hpp"
#include "Hooks.hpp"

namespace menu {

    void Init() {
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;

        ImGui::SetNextWindowSize(ImVec2(500, 600), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(25, 25), ImGuiCond_FirstUseEver);

        ImGui::Begin("Silent Hill F Speedrunning Tool. Made by CactusDuper", &GIsGUIOpen, flags);

        if (ImGui::CollapsingHeader("Drawing")) {
            if (ImGui::TreeNode("Triggers")) {
                if (ImGui::Checkbox("Draw Trigger Volumes", &GDrawCustomTriggers)) {
                    GShouldUpdateCustomTriggersVisibility = true;
                }
                ImGui::Checkbox("Draw Trigger Info", &GDrawCustomTriggersNames);
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Shapes")) {
                if (ImGui::Checkbox("Draw Shapes", &GDrawShapes)) {
                    GShouldUpdateShapesVisibility = true;
                }
                ImGui::Checkbox("Draw Shape Names", &GDrawShapesNames);
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Collision")) {
                if (ImGui::Checkbox("Draw All Collision Meshes", &GDrawAllCollisions)) {
                    GShouldUpdateAllCollisions = true;
                }
                ImGui::Checkbox("Draw Collision Info", &GDrawCollisionNames);
                ImGui::Separator();
                if (ImGui::Checkbox("Enable Collision View (Hides non collision Meshes, WIP)", &GCollisionViewActive)) {
                    GShouldUpdateCollisionView = true;
                }
                ImGui::TextWrapped("Note: Might break stuff.");
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Characters")) {
                ImGui::Checkbox("Draw Character Info", &GDrawCharacters);
                ImGui::Separator();

                if (GDrawCharacters && ImGui::TreeNode("Individual Character Filter")) {
                    // Sync before
                    UpdateCharacterSelectionList();

                    if (ImGui::Button("Select All")) {
                        for (auto& pair : GCharacterSelectionMap) {
                            pair.second.bIsVisible = true;
                        }
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Deselect All")) {
                        for (auto& pair : GCharacterSelectionMap) {
                            pair.second.bIsVisible = false;
                        }
                    }

                    ImGui::Separator();

                    for (auto& [index, info] : GCharacterSelectionMap) {
                        std::string name_utf8;
                        int size_needed = WideCharToMultiByte(CP_UTF8, 0, info.Name.c_str(), (int)info.Name.length(), NULL, 0, NULL, NULL);
                        name_utf8.resize(size_needed);
                        WideCharToMultiByte(CP_UTF8, 0, info.Name.c_str(), (int)info.Name.length(), &name_utf8[0], size_needed, NULL, NULL);

                        ImGui::Checkbox(name_utf8.c_str(), &info.bIsVisible);
                    }

                    ImGui::TreePop();
                }

                if (GDrawCharacters && ImGui::TreeNode("Display Settings")) {
                    ImGui::Text("Master Toggles");
                    ImGui::Checkbox("Base Info##Master", &GCharDisplaySettings.bShowBaseInfo); ImGui::SameLine();
                    ImGui::Checkbox("Enemy Info##Master", &GCharDisplaySettings.bShowEnemyInfo); ImGui::SameLine();
                    ImGui::Checkbox("EmBase Info##Master", &GCharDisplaySettings.bShowEmBaseInfo); ImGui::SameLine();
                    ImGui::Checkbox("Component Info##Master", &GCharDisplaySettings.bShowComponentInfo);

                    if (ImGui::TreeNode("ANoceCharacter")) {
                        ImGui::Checkbox("Character Name", &GCharDisplaySettings.bShowCharacterName);
                        ImGui::Checkbox("Character Tag", &GCharDisplaySettings.bShowCharacterTag);
                        ImGui::Checkbox("Level", &GCharDisplaySettings.bShowCharacterLevel);
                        ImGui::Checkbox("Alive Status", &GCharDisplaySettings.bShowAliveStatus);
                        ImGui::Checkbox("Die Count", &GCharDisplaySettings.bShowDieCount);
                        ImGui::Checkbox("Movement Mode", &GCharDisplaySettings.bShowMovementMode);
                        ImGui::Checkbox("Significance", &GCharDisplaySettings.bShowSignificance);
                        ImGui::Checkbox("Activated by Distance", &GCharDisplaySettings.bShowActivatedByDistance);
                        ImGui::Checkbox("Spawner", &GCharDisplaySettings.bShowSpawner);
                        ImGui::Checkbox("Territory", &GCharDisplaySettings.bShowTerritory);
                        ImGui::Checkbox("Under Attack Timers", &GCharDisplaySettings.bShowUnderAttackTimer);
                        ImGui::Checkbox("Time Since Spawn", &GCharDisplaySettings.bShowGameTimeSinceSpawn);
                        ImGui::Checkbox("Ability/Effect Count", &GCharDisplaySettings.bShowAbilityInfo);
                        ImGui::TreePop();
                    }
                    if (ImGui::TreeNode("Last Damage")) {
                        ImGui::Checkbox("Show Section", &GCharDisplaySettings.bShowLastDamage);
                        ImGui::Checkbox("Instigator", &GCharDisplaySettings.bShowDamageInstigator);
                        ImGui::Checkbox("Combo", &GCharDisplaySettings.bShowDamageCombo);
                        ImGui::Checkbox("Health Dmg", &GCharDisplaySettings.bShowHealthDamage);
                        ImGui::Checkbox("Wince Dmg", &GCharDisplaySettings.bShowWinceDamage);
                        ImGui::Checkbox("Wince Type", &GCharDisplaySettings.bShowWinceType);
                        ImGui::TreePop();
                    }
                    if (ImGui::TreeNode("ANoceEnemyCharacter")) {
                        ImGui::Checkbox("Is Fake Dead", &GCharDisplaySettings.bShowIsFakeDead);
                        ImGui::Checkbox("Is Unkillable", &GCharDisplaySettings.bShowUnkillable);
                        ImGui::Checkbox("Force In Sight", &GCharDisplaySettings.bShowForceInSight);
                        ImGui::Checkbox("Hit Perform Group", &GCharDisplaySettings.bShowHitPerformGroup);
                        ImGui::Checkbox("Fake Dead Count", &GCharDisplaySettings.bShowFakeDeadCount);
                        ImGui::Checkbox("Revive Health Ratio", &GCharDisplaySettings.bShowReviveHealthRatio);
                        ImGui::Checkbox("Extend Aim Distance", &GCharDisplaySettings.bShowExtendAimDistance);
                        ImGui::Checkbox("Fake Dead Timers", &GCharDisplaySettings.bShowFakeDeadTimers);
                        ImGui::Checkbox("Is Visible To Player", &GCharDisplaySettings.bShowIsVisibleToPlayer);
                        ImGui::TreePop();
                    }
                    if (ImGui::TreeNode("Enemy Attribute Set")) {
                        ImGui::Checkbox("Show Section", &GCharDisplaySettings.bShowEnemyAttributes);
                        ImGui::Checkbox("Health Attack", &GCharDisplaySettings.bShowHealthAttack);
                        ImGui::Checkbox("Wince Attack", &GCharDisplaySettings.bShowWinceAttack);
                        ImGui::Checkbox("Damage Ratio Health", &GCharDisplaySettings.bShowDmgRatioHealth);
                        ImGui::Checkbox("Damage Ratio Wince", &GCharDisplaySettings.bShowDmgRatioWince);
                        ImGui::Checkbox("Jealousy", &GCharDisplaySettings.bShowJealous);
                        ImGui::Checkbox("Link Attack Ratio", &GCharDisplaySettings.bShowLinkAttackRatio);
                        ImGui::Checkbox("Fake Dead Wakeup Ratio", &GCharDisplaySettings.bShowFakeDeadWakeupRatio);
                        ImGui::TreePop();
                    }
                    if (ImGui::TreeNode("EmBase Character (BP)")) {
                        ImGui::Checkbox("Is Strafing", &GCharDisplaySettings.bShowIsStrafe);
                        ImGui::Checkbox("Hope Not To Move", &GCharDisplaySettings.bShowHopeNotToMove);
                        ImGui::Checkbox("Enable AI On Damaged", &GCharDisplaySettings.bShowEnableAIOnDamaged);
                        ImGui::Checkbox("Movement Speeds", &GCharDisplaySettings.bShowMovementSpeeds);
                        ImGui::Checkbox("Nav Agent Type", &GCharDisplaySettings.bShowNavAgent);
                        ImGui::Checkbox("Is Optimized", &GCharDisplaySettings.bShowIsOptimized);
                        ImGui::Checkbox("Show Turn Montages", &GCharDisplaySettings.bShowTurnMontages);
                        ImGui::Checkbox("Default Capsule Info", &GCharDisplaySettings.bShowDefaultCapsuleInfo);
                        ImGui::TreePop();
                    }
                    if (ImGui::TreeNode("Component Details")) {
                        if (ImGui::TreeNode("AI Controller")) {
                            ImGui::Checkbox("Show Section", &GCharDisplaySettings.bShowAIController);
                            ImGui::Checkbox("Alert Type", &GCharDisplaySettings.bShowAIAlertType);
                            ImGui::Checkbox("Alertness", &GCharDisplaySettings.bShowAIAlertness);
                            ImGui::Checkbox("Think Status", &GCharDisplaySettings.bShowAIThinkStatus);
                            ImGui::Checkbox("Hated Target", &GCharDisplaySettings.bShowAIHatestTarget);
                            ImGui::Checkbox("Path Status", &GCharDisplaySettings.bShowAIPathStatus);
                            ImGui::TreePop();
                        }
                        if (ImGui::TreeNode("Hit Perform Component")) {
                            ImGui::Checkbox("Show Section", &GCharDisplaySettings.bShowHitPerformComponent);
                            ImGui::Checkbox("Data Asset", &GCharDisplaySettings.bShowHitPerformAsset);
                            ImGui::Checkbox("Hit Move Info", &GCharDisplaySettings.bShowHitMoveInfo);
                            ImGui::TreePop();
                        }
                        if (ImGui::TreeNode("Attack & Damage Components (WIP)")) {
                            ImGui::Checkbox("Show AtkInfo Section (WIP)", &GCharDisplaySettings.bShowAttackInfoComponent);
                            ImGui::Checkbox("Show DmgHandle Section (WIP)", &GCharDisplaySettings.bShowDamageHandleComponent);
                            ImGui::Checkbox("Show BodyPart Section (WIP)", &GCharDisplaySettings.bShowBodyPartGroupComponent);
                            ImGui::Checkbox("Show AtkTrace Section (WIP)", &GCharDisplaySettings.bShowAttackTraceComponent);
                            ImGui::TreePop();
                        }
                        if (ImGui::TreeNode("Nav Link Component")) {
                            ImGui::Checkbox("Show Section", &GCharDisplaySettings.bShowNavLinkInfo);
                            ImGui::TreePop();
                        }
                        ImGui::TreePop();
                    }
                    ImGui::TreePop();
                }
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("FSM")) {
                ImGui::Checkbox("Draw FSM Info", &GDrawFSMInfo);
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Player Info")) {
                ImGui::Checkbox("Draw Player Info", &GDrawPlayerStatus);
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Progress and Extra Info")) {
                ImGui::Checkbox("Draw Progress and Extra Info", &GDrawProgressAndPlayerStateExtra);
                ImGui::TreePop();
            }
            ImGui::Separator();
            ImGui::SliderFloat("Draw Distance (larger = slower!)", &GDistanceToDraw, 100.0f, 30000.0f, "%.0f");
        }

        if (ImGui::CollapsingHeader("Menu Settings")) {
            ImGui::Text("Menu Key: DELETE");
            if (ImGui::Button("Rescan (if things are no longer showing)")) {
                SetupPlayer();
            }
        }

        ImGui::End();
    }
}