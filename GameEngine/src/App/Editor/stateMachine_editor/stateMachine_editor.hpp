#pragma once
#include <string>
#include <memory>
#include "imgui.h"

#include "script_generator.hpp"
#include "component/components/state_machine_component.hpp"
#include "editor/eno_editor.hpp"

namespace Editor {

    class StateMachineEditor {
    public:
        StateMachineEditor(EnoEditor& editor)
            : scriptManager_(editor.getScriptManager()), sceneTree_(editor.getSceneTree()), entMng_(editor.getEntityManager()) {
        }

        // Main editor UI for state machine component
        void renderStateMachineUI(Editor::Node* node, ECSComponents::StateMachineComponent& smComp) {
            if (!node) return;

            ImGui::PushID(node->sceneObjectID_);

            // Script attachment section
            renderScriptSection(node);

            ImGui::Separator();

            // State list section
            renderStateList(node, smComp);

            ImGui::Separator();

            // Add new state section
            renderAddStateSection(node, smComp);

            ImGui::PopID();
        }

    private:
        Script::EnoScriptManager& scriptManager_;
        EnoSceneTree& sceneTree_;
        ECS::EntityManager& entMng_;
        char newStateName_[64] = "";
        char newScriptName_[64] = "";
        std::string warningBuf_ = "";
        bool showScriptDialog_ = false;

        // Render script attachment UI
        void renderScriptSection(Editor::Node* node) {
            ImGui::Text("Script Attachment");
            ImGui::Spacing();

            if (node->script_) {
                // Script is attached
                std::string scriptName(node->script_->name_.begin(), node->script_->name_.end());
                ImGui::Text("Attached Script: %s", scriptName.c_str());

                if (ImGui::Button("Open Script")) {
                    std::string scriptPath = node->script_->path();
                    scriptManager_.openScript(scriptPath);
                }

                ImGui::SameLine();

                if (ImGui::Button("Detach Script")) {
                    sceneTree_.detachScript(node);
                }
            }
            else {
                // No script attached
                ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.0f, 1.0f), "No script attached");
                ImGui::Spacing();

                if (ImGui::Button("Generate New Script")) {
                    showScriptDialog_ = true;
                    snprintf(newScriptName_, sizeof(newScriptName_), "%s_Script", node->name_.c_str());
                }

                ImGui::SameLine();

                if (ImGui::Button("Attach Existing Script")) {
                    renderScriptSelectionPopup(node);
                }

                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.0f, 1.0f), "Add script to use state machine");
            }

            // Script creation dialog
            if (showScriptDialog_) {
                renderScriptCreationDialog(node);
            }
        }

        // Render list of states
        void renderStateList(Editor::Node* node, ECSComponents::StateMachineComponent& smComp) {
            ImGui::Text("States (%d/%d)", smComp.numStates, ECSComponents::MAX_STATES);
            ImGui::Spacing();

            if (smComp.numStates == 0) {
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "No states defined");
                return;
            }

            // Display current state
            ImGui::Text("Current State: %s", smComp.currentState);
            ImGui::Spacing();

            // State list with options
            for (int i = 0; i < smComp.numStates; ++i) {
                ImGui::PushID(i);

                bool isCurrent = strcmp(smComp.states[i].name,
                    smComp.currentState) == 0;

                if (isCurrent) {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
                }

                ImGui::BulletText("%s", smComp.states[i].name);

                if (isCurrent) {
                    ImGui::PopStyleColor();
                }

                // Show method names
                ImGui::Indent();
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Enter: %s()", smComp.states[i].onEnterMethod);
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Update: %s()", smComp.states[i].onUpdateMethod);
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Exit: %s()", smComp.states[i].onExitMethod);
                ImGui::Unindent();

                // Action buttons
                ImGui::SameLine();
                if (ImGui::SmallButton("Set Current")) {
                    setCurrentState(smComp, smComp.states[i].name);
                }

                ImGui::SameLine();
                if (ImGui::SmallButton("Remove")) {
                    removeState(node, smComp, smComp.states[i].name, i);
                }

                ImGui::Spacing();
                ImGui::PopID();
            }
        }

        // Render add state section
        void renderAddStateSection(Editor::Node* node,
            ECSComponents::StateMachineComponent& smComp) {
            ImGui::Text("Add New State");
            ImGui::Spacing();

            if (!node->script_) {
                ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.0f, 1.0f), "Attach a script first to add states");
                return;
            }

            if (smComp.numStates >= ECSComponents::MAX_STATES) {
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Maximum states reached!");
                return;
            }

            ImGui::InputText("State Name", newStateName_, sizeof(newStateName_));

            if (ImGui::Button("Add State")) {
                if (!newStateName_ || newStateName_[0] == '\0') {
                    warningBuf_ = "Empty name not allowed!";
                }
                else {
                    addState(node, smComp, newStateName_);
                    newStateName_[0] = '\0';
                }
            }

            if (!warningBuf_.empty())
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), warningBuf_.c_str());
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Methods will be auto-generated");
        }

        // Add a new state
        void addState(Editor::Node* node, ECSComponents::StateMachineComponent& smComp, const char* stateName) {
            // to debug
            std::cout << "Add state..." << std::endl;
            if (smComp.addState(stateName)) {
                // Auto-generate methods in script if script is attached
                if (node->script_) {
                    std::string scriptPath = node->script_->path();
                    std::cout << "Script path: " << scriptPath << std::endl;
                    Script::StateMachineScriptGenerator::generateStateMethods(scriptPath, stateName);
                }

                // Set as current state if it's the first state
                if (smComp.numStates == 1) {
                    strncpy(smComp.currentState, stateName, sizeof(smComp.currentState) - 1);
                    smComp.currentState[sizeof(smComp.currentState) - 1] = '\0';
                }
            }
        }

        // Remove a state
        void removeState(Editor::Node* node, ECSComponents::StateMachineComponent& smComp,
            const char* stateName, int index) {
            // Remove methods from script if attached
            if (node->script_) {
                std::string scriptPath = node->script_->path();
                Script::StateMachineScriptGenerator::removeStateMethods(scriptPath, stateName);
            }
            smComp.removeState(stateName);
        }

        // Set current state
        void setCurrentState(ECSComponents::StateMachineComponent& smComp, const char* stateName) {
            strncpy(smComp.currentState, stateName, sizeof(smComp.currentState) - 1);
            smComp.currentState[sizeof(smComp.currentState) - 1] = '\0';
        }

        // Render script selection popup
        void renderScriptSelectionPopup(Editor::Node* node) {
            ImGui::OpenPopup("Select Script");

            if (ImGui::BeginPopup("Select Script")) {
                ImGui::Text("Available Scripts:");
                ImGui::Separator();

                std::vector<std::wstring> scripts = scriptManager_.getAvailableScripts();

                for (const auto& scriptName : scripts) {
                    std::string scriptNameStr(scriptName.begin(), scriptName.end());

                    if (ImGui::Selectable(scriptNameStr.c_str())) {
                        attachScript(node, scriptName);
                    }
                }

                ImGui::EndPopup();
            }
        }

        // Render script creation dialog
        void renderScriptCreationDialog(Editor::Node* node) {
            ImGui::OpenPopup("Create Script");

            if (ImGui::BeginPopupModal("Create Script", &showScriptDialog_)) {
                ImGui::Text("Enter script name:");
                ImGui::InputText("##scriptname", newScriptName_,
                    sizeof(newScriptName_));

                if (ImGui::Button("Create")) {
                    if (strlen(newScriptName_) > 0) {
                        createAndAttachScript(node, newScriptName_);
                        showScriptDialog_ = false;
                    }
                }

                ImGui::SameLine();

                if (ImGui::Button("Cancel")) {
                    showScriptDialog_ = false;
                }

                ImGui::EndPopup();
            }
        }

        // Create and attach a new script
        void createAndAttachScript(Editor::Node* node, const char* scriptName) {
            try {
                auto script = scriptManager_.createAndOpenScript(scriptName);

                std::cout << "successfully created script..." << std::endl;

                if (script) {
                    sceneTree_.attachScript(node, std::move(script));

                    // Generate state methods for existing states
                    if (node->entity_.has<ECSComponents::StateMachineComponent>(entMng_)) {
                        auto& smComp = node->entity_.get<ECSComponents::StateMachineComponent>(entMng_);
                        std::string scriptPath = node->script_->path();

                        for (int i = 0; i < smComp.numStates; ++i) {
                            Script::StateMachineScriptGenerator::generateStateMethods(scriptPath, smComp.states[i].name);
                        }
                    }
                }
            }
            catch (const std::exception& e) {
                // Handle error
                ImGui::OpenPopup("Error");
            }
        }

        // Attach an existing script
        void attachScript(Editor::Node* node, const std::wstring& scriptName) {
            try {
                auto script = scriptManager_.loadScript(scriptName);

                if (script) {
                    sceneTree_.attachScript(node, std::move(script));

                    // Generate state methods for existing states
                    if (node->entity_.has<ECSComponents::StateMachineComponent>(entMng_)) {
                        auto& smComp = node->entity_.get<ECSComponents::StateMachineComponent>(entMng_);
                        std::string scriptPath = node->script_->path();

                        for (int i = 0; i < smComp.numStates; ++i) {
                            Script::StateMachineScriptGenerator::generateStateMethods(scriptPath, smComp.states[i].name);
                        }
                    }
                }
            }
            catch (const std::exception& e) {
                // Handle error
                ImGui::OpenPopup("Error");
            }
        }
    };
}