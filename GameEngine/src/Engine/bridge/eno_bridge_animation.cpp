/*============================================================================================
* @file         eno_bridge_animation.cpp
* @brief        Animation component bridge — set/get the current animation.
*
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
*//*========================================================================================*/

#include "eno_bridge.hpp"
#include "../component/components/drawInfo.hpp"

namespace Bridge
{
    static ECSComponents::AnimationControllerComponent* getAnimCtrl(uint32_t sceneObjectID)
    {
        if (!gEngine) return nullptr;
        Editor::Node* node = gEngine->getEnoEditor().getSceneTree().getNodeBySceneObjectID(sceneObjectID);
        if (!node) return nullptr;
        if (!node->entity_.has<ECSComponents::AnimationControllerComponent>(gEngine->getEntityManager())) return nullptr;
        return &node->entity_.get<ECSComponents::AnimationControllerComponent>(gEngine->getEntityManager());
    }

    extern "C"
    {
        ENGINE_API void __stdcall SetAnimation(uint32_t sceneObjectID, const char* animName)
        {
            if (auto* ac = getAnimCtrl(sceneObjectID))
            {
                strncpy_s(ac->currentAnimationName, animName, sizeof(ac->currentAnimationName) - 1);
                ac->currentAnimationName[sizeof(ac->currentAnimationName) - 1] = '\0';
                ac->updateSerializationFields();
            }
        }

        ENGINE_API const char* __stdcall GetCurrentAnimation(uint32_t sceneObjectID)
        {
            auto* ac = getAnimCtrl(sceneObjectID);
            return ac ? ac->currentAnimationName : "";
        }
    }
}
