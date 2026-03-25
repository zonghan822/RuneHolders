/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         state_machine_component.hpp
* @author       Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   50%
* @co-author    ZongHan Law (zonghan.law@digipen.edu)   50%
* @brief        See file name.
*//*========================================================================================*/

#pragma once
#include <cstring>

namespace ECSComponents
{
    constexpr int MAX_STATES = 16;
    constexpr int MAX_METHOD_NAME = 64;

    // State info with method names for each callback
    struct StateInfo {
        char name[32];
        char onEnterMethod[MAX_METHOD_NAME];
        char onUpdateMethod[MAX_METHOD_NAME];
        char onExitMethod[MAX_METHOD_NAME];

        StateInfo() {
            name[0] = '\0';
            onEnterMethod[0] = '\0';
            onUpdateMethod[0] = '\0';
            onExitMethod[0] = '\0';
        }

        // Auto-generate method names based on state name
        void generateMethodNames() {
            if (name[0] != '\0') {
                // Generate: statename_enter(), statename_update(), statename_exit()
                snprintf(onEnterMethod, MAX_METHOD_NAME, "%s_enter", name);
                snprintf(onUpdateMethod, MAX_METHOD_NAME, "%s_update", name);
                snprintf(onExitMethod, MAX_METHOD_NAME, "%s_exit", name);
            }
        }
    };

    struct StateMachineComponent
    {
        StateInfo states[MAX_STATES];
        int numStates = 0;
        char currentState[32] = "None";
        float timeInState = 0.0f;

        // Add a new state and auto-generate method names
        bool addState(const char* stateName) {
            if (numStates >= MAX_STATES) {
                return false;
            }

            // Check if state already exists
            for (int i = 0; i < numStates; ++i) {
                if (strcmp(states[i].name, stateName) == 0) {
                    return false; // State already exists
                }
            }

            // Add new state
            strncpy(states[numStates].name, stateName, sizeof(states[numStates].name) - 1);
            states[numStates].name[sizeof(states[numStates].name) - 1] = '\0';
            states[numStates].generateMethodNames();

            ++numStates;
            return true;
        }

        // Remove a state by name
        bool removeState(const char* stateName) {
            for (int i = 0; i < numStates; ++i) {
                if (strcmp(states[i].name, stateName) == 0) {
                    // Shift remaining states down
                    for (int j = i; j < numStates - 1; ++j) {
                        states[j] = states[j + 1];
                    }
                    --numStates;

                    // Reset current state if it was removed
                    if (strcmp(currentState, stateName) == 0) {
                        strcpy(currentState, "None");
                    }
                    return true;
                }
            }
            return false;
        }

        // Get state info by name
        StateInfo* getState(const char* stateName) {
            for (int i = 0; i < numStates; ++i) {
                if (strcmp(states[i].name, stateName) == 0) {
                    return &states[i];
                }
            }
            return nullptr;
        }

        // When true, requestStateChange calls are silently ignored for this entity
        bool stateChangeLocked = false;

        // Signals
        std::unordered_map<std::string, Events::EnoSignal> signals_;

        template <typename Visitor>
        void reflect(Visitor&& v)
        {
            v("NumStates", numStates, "int");
            v("CurrentState", currentState, "char32");
            v("TimeInState", timeInState, "float");

            for (int i = 0; i < numStates; ++i) {
                char prefix[64];
                snprintf(prefix, sizeof(prefix), "State[%d]", i);

                char key[128];
                snprintf(key, sizeof(key), "%s.name", prefix);
                v(key, states[i].name, "char32");

                snprintf(key, sizeof(key), "%s.onEnter", prefix);
                v(key, states[i].onEnterMethod, "char64");

                snprintf(key, sizeof(key), "%s.onUpdate", prefix);
                v(key, states[i].onUpdateMethod, "char64");

                snprintf(key, sizeof(key), "%s.onExit", prefix);
                v(key, states[i].onExitMethod, "char64");
            }
        }

        template <typename Visitor>
        void reflect(Visitor&& v) const
        {
            v("NumStates", numStates, "int");
            v("CurrentState", currentState, "char32");
            v("TimeInState", timeInState, "float");

            for (int i = 0; i < numStates; ++i) {
                char prefix[64];
                snprintf(prefix, sizeof(prefix), "State[%d]", i);

                char key[128];
                snprintf(key, sizeof(key), "%s.name", prefix);
                v(key, states[i].name, "char32");

                snprintf(key, sizeof(key), "%s.onEnter", prefix);
                v(key, states[i].onEnterMethod, "char64");

                snprintf(key, sizeof(key), "%s.onUpdate", prefix);
                v(key, states[i].onUpdateMethod, "char64");

                snprintf(key, sizeof(key), "%s.onExit", prefix);
                v(key, states[i].onExitMethod, "char64");
            }
        }
    };
}