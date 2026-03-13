/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         eno_bridge_animation.cpp
* @author       ZongHan Law (zonghan.law@digipen.edu)
* @brief        Animation component bridge — set/get the current animation. All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
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

        ENGINE_API void __stdcall SetAnimationLooping(uint32_t sceneObjectID, bool loop)
        {
            auto* ac = getAnimCtrl(sceneObjectID);
            if (!ac) return;

            for (int i = 0; i < ac->animationCount; ++i)
            {
                if (strcmp(ac->animations[i].name, ac->currentAnimationName) == 0)
                {
                    ac->animations[i].looping = loop;
                    break;
                }
            }
            ac->updateSerializationFields();

            auto* ctrl = gEngine->getAnimationRegistry().get(ac->controllerID);
            if (ctrl && ctrl->getCurrentAnimation())
                ctrl->getCurrentAnimation()->setLooping(loop);
        }

        ENGINE_API void __stdcall SetAnimationFrameDuration(uint32_t sceneObjectID, float frameDuration)
        {
            auto* ac = getAnimCtrl(sceneObjectID);
            if (!ac) return;

            for (int i = 0; i < ac->animationCount; ++i)
            {
                if (strcmp(ac->animations[i].name, ac->currentAnimationName) == 0)
                {
                    ac->animations[i].frameDuration = frameDuration;
                    break;
                }
            }
            ac->updateSerializationFields();

            auto* ctrl = gEngine->getAnimationRegistry().get(ac->controllerID);
            if (ctrl && ctrl->getCurrentAnimation())
                for (auto& frame : ctrl->getCurrentAnimation()->getFrames())
                    frame.duration = frameDuration;
        }

        ENGINE_API uint32_t __stdcall GetAnimationFrameCount(uint32_t sceneObjectID)
        {
            auto* ac = getAnimCtrl(sceneObjectID);
            if (!ac) return 0;

            auto* ctrl = gEngine->getAnimationRegistry().get(ac->controllerID);
            if (ctrl && ctrl->getCurrentAnimation())
                return static_cast<uint32_t>(ctrl->getCurrentAnimation()->getFrameCount());
            return 0;
        }
    }
}
