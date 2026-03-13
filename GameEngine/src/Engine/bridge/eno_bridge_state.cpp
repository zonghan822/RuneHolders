/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         eno_bridge_state.cpp
* @author       ZongHan Law (zonghan.law@digipen.edu)
* @brief        State machine bridge — request state transitions. All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
*//*========================================================================================*/

#include "eno_bridge.hpp"
#include "../component/components/state_machine_component.hpp"

namespace Bridge
{
    extern "C"
    {
        ENGINE_API void __stdcall ChangeState(uint32_t sceneObjectID, const wchar_t* stateName)
        {
            if (!gEngine) return;

            std::wstring wState(stateName);
            std::string state(wState.begin(), wState.end());

            auto& sceneTree = gEngine->getEnoEditor().getSceneTree();
            Editor::Node* node = sceneTree.getNodeBySceneObjectID(sceneObjectID);
            if (!node)
            {
                std::cerr << "[Bridge] ChangeState: no node for sceneObjectID " << sceneObjectID << '\n';
                return;
            }

            gEngine->getStateMachineSys().requestStateChange(node->entity_.id(), state);
        }
    }
}
