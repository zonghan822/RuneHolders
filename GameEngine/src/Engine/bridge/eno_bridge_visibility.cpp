/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         eno_bridge_visibility.cpp
* @author       Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   67%
* @co-author    ZongHan Law (zonghan.law@digipen.edu)   33%
* @brief        Visibility component bridge — z-index and visible flag.
*//*========================================================================================*/



#include "eno_bridge.hpp"
#include "../component/components/visibility.hpp"

namespace Bridge
{
    static ECSComponents::VisibilityComponent* getVisibility(uint32_t sceneObjectID)
    {
        if (!gEngine) return nullptr;
        Editor::Node* node = gEngine->getEnoEditor().getSceneTree().getNodeBySceneObjectID(sceneObjectID);
        if (!node) return nullptr;
        if (!node->entity_.has<ECSComponents::VisibilityComponent>(gEngine->getEntityManager())) return nullptr;
        return &node->entity_.get<ECSComponents::VisibilityComponent>(gEngine->getEntityManager());
    }

    extern "C"
    {
        ENGINE_API uint32_t __stdcall GetZIndex(uint32_t sceneObjectID)
        {
            auto* v = getVisibility(sceneObjectID);
            return v ? v->z_index : 0;
        }

        ENGINE_API void __stdcall SetZIndex(uint32_t sceneObjectID, uint32_t z)
        {
            if (auto* v = getVisibility(sceneObjectID))
                v->z_index = z;
        }

        ENGINE_API bool __stdcall GetVisibility(uint32_t sceneObjectID)
        {
            auto* v = getVisibility(sceneObjectID);
            return v ? v->visible : true;
        }

        ENGINE_API void __stdcall SetVisibility(uint32_t sceneObjectID, bool visible)
        {
            if (auto* v = getVisibility(sceneObjectID))
                v->visible = visible;
        }
    }
}
