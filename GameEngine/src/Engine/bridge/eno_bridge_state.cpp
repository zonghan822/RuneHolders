/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         eno_bridge_state.cpp
* @author       Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   67%
* @co-author    ZongHan Law (zonghan.law@digipen.edu)   33%
* @brief        State machine bridge — request state transitions.
*//*========================================================================================*/



#include "eno_bridge.hpp"
#include "../component/components/state_machine_component.hpp"
#include <algorithm>

namespace Bridge
{
    extern "C"
    {
        ENGINE_API void __stdcall ChangeState(uint32_t sceneObjectID, const wchar_t* stateName)
        {
            if (!gEngine) return;

            std::wstring wState(stateName);
            std::string state(wState.size(), '\0');
            std::transform(wState.begin(), wState.end(), state.begin(),
                [](wchar_t c) { return static_cast<char>(c); });

            auto& sceneTree = gEngine->getEnoEditor().getSceneTree();
            Editor::Node* node = sceneTree.getNodeBySceneObjectID(sceneObjectID);
            if (!node)
            {
                std::cerr << "[Bridge] ChangeState: no node for sceneObjectID " << sceneObjectID << '\n';
                return;
            }

            gEngine->getStateMachineSys().requestStateChange(node->entity_.id(), state);
        }

        // Returns true if state transitions are currently disabled for an entity.
        ENGINE_API bool __stdcall GetStateChangeLocked(uint32_t sceneObjectID)
        {
            if (!gEngine) return false;

            auto& sceneTree = gEngine->getEnoEditor().getSceneTree();
            Editor::Node* node = sceneTree.getNodeBySceneObjectID(sceneObjectID);
            if (!node) return false;

            auto& entityManager = gEngine->getEntityManager();
            if (!node->entity_.has<ECSComponents::StateMachineComponent>(entityManager)) return false;

            return node->entity_.get<ECSComponents::StateMachineComponent>(entityManager).stateChangeLocked;
        }

        // Enable or disable state transitions for an entity.
        ENGINE_API void __stdcall SetStateChangeLocked(uint32_t sceneObjectID, bool locked)
        {
            if (!gEngine) return;

            auto& sceneTree = gEngine->getEnoEditor().getSceneTree();
            Editor::Node* node = sceneTree.getNodeBySceneObjectID(sceneObjectID);
            if (!node) return;

            auto& entityManager = gEngine->getEntityManager();
            if (!node->entity_.has<ECSComponents::StateMachineComponent>(entityManager)) return;

            node->entity_.get<ECSComponents::StateMachineComponent>(entityManager).stateChangeLocked = locked;
        }

        // Copy the current state name into outBuffer (null-terminated, ANSI).
        ENGINE_API void __stdcall GetCurrentState(uint32_t sceneObjectID, char* outBuffer, int bufferSize)
        {
            if (!gEngine || !outBuffer || bufferSize <= 0) return;

            auto& sceneTree = gEngine->getEnoEditor().getSceneTree();
            Editor::Node* node = sceneTree.getNodeBySceneObjectID(sceneObjectID);
            if (!node) { outBuffer[0] = '\0'; return; }

            auto& entityManager = gEngine->getEntityManager();
            if (!node->entity_.has<ECSComponents::StateMachineComponent>(entityManager)) { outBuffer[0] = '\0'; return; }

            const char* state = node->entity_.get<ECSComponents::StateMachineComponent>(entityManager).currentState;
            strncpy_s(outBuffer, bufferSize, state, bufferSize - 1);
        }
    }
}
