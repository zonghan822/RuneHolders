/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         eno_bridge_button.cpp
* @author       Caven Koh (caven.koh@digipen.edu)   75%
* @co-author    ZongHan Law (zonghan.law@digipen.edu)   25%
* @brief        Button Bridge - Access and mutate button activity
*//*========================================================================================*/



#include "eno_bridge.hpp"
#include "../component/components/buttons.hpp"

namespace Bridge
{
    static ECSComponents::ButtonComponent* getButton(uint32_t sceneObjectID)
    {
        if (!gEngine) return nullptr;
        Editor::Node* node = gEngine->getEnoEditor().getSceneTree().getNodeBySceneObjectID(sceneObjectID);
        if (!node) return nullptr;
        if (!node->entity_.has<ECSComponents::ButtonComponent>(gEngine->getEntityManager())) return nullptr;
        return &node->entity_.get<ECSComponents::ButtonComponent>(gEngine->getEntityManager());
    }

    extern "C"
    {
        ENGINE_API bool __stdcall GetButtonActive(uint32_t sceneObjectID)
        {
            auto* b = getButton(sceneObjectID);
            return b ? b->getActive() : false;
        }

        ENGINE_API void __stdcall SetButtonActive(uint32_t sceneObjectID, bool active)
        {
            if (auto* b = getButton(sceneObjectID))
                b->getActive() = active;
        }
    }
}
