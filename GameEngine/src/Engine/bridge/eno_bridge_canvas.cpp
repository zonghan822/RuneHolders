/*============================================================================================
* @file         eno_bridge_canvas.cpp
* @brief        
*
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
*//*========================================================================================*/

#include "eno_bridge.hpp"
#include "../component/components/canvas.hpp"

namespace Bridge
{
    static ECSComponents::CanvasComponent* getCanvas(uint32_t sceneObjectID)
    {
        if (!gEngine) return nullptr;
        Editor::Node* node = gEngine->getEnoEditor().getSceneTree().getNodeBySceneObjectID(sceneObjectID);
        if (!node) return nullptr;
        if (!node->entity_.has<ECSComponents::CanvasComponent>(gEngine->getEntityManager())) return nullptr;
        return &node->entity_.get<ECSComponents::CanvasComponent>(gEngine->getEntityManager());
    }

    extern "C"
    {
        ENGINE_API bool __stdcall GetCanvasActive(uint32_t sceneObjectID)
        {
            auto* c = getCanvas(sceneObjectID);
            return c ? c->getActive() : true;
        }

        ENGINE_API void __stdcall SetCanvasActive(uint32_t sceneObjectID, bool active)
        {
            if (auto* c = getCanvas(sceneObjectID))
                c->setActive(active);
        }
    }
}
