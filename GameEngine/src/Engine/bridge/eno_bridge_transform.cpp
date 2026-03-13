/*============================================================================================
* @file         eno_bridge_transform.cpp
* @brief        Transform component bridge — position, rotation, scale.
*
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
*//*========================================================================================*/

#include "eno_bridge.hpp"
#include "../component/components/transform.hpp"

namespace Bridge
{
    // Returns the TransformComponent for a scene object, or nullptr if not found.
    static ECSComponents::TransformComponent* getTransform(uint32_t sceneObjectID)
    {
        if (!gEngine) return nullptr;
        Editor::Node* node = gEngine->getEnoEditor().getSceneTree().getNodeBySceneObjectID(sceneObjectID);
        if (!node) return nullptr;
        if (!node->entity_.has<ECSComponents::TransformComponent>(gEngine->getEntityManager())) return nullptr;
        return &node->entity_.get<ECSComponents::TransformComponent>(gEngine->getEntityManager());
    }

    extern "C"
    {
        // ============= Position =============

        ENGINE_API float __stdcall GetPositionX(uint32_t sceneObjectID)
        {
            auto* t = getTransform(sceneObjectID);
            return t ? t->pos.x : 0.0f;
        }

        ENGINE_API float __stdcall GetPositionY(uint32_t sceneObjectID)
        {
            auto* t = getTransform(sceneObjectID);
            return t ? t->pos.y : 0.0f;
        }

        ENGINE_API void __stdcall SetPosition(uint32_t sceneObjectID, float x, float y)
        {
            if (auto* t = getTransform(sceneObjectID))
            {
                t->pos.x = x;
                t->pos.y = y;
            }
        }

        // ============= Rotation =============

        ENGINE_API float __stdcall GetRotationX(uint32_t sceneObjectID)
        {
            auto* t = getTransform(sceneObjectID);
            return t ? t->rot.x : 0.0f;
        }

        ENGINE_API float __stdcall GetRotationY(uint32_t sceneObjectID)
        {
            auto* t = getTransform(sceneObjectID);
            return t ? t->rot.y : 0.0f;
        }

        ENGINE_API void __stdcall SetRotation(uint32_t sceneObjectID, float x, float y)
        {
            if (auto* t = getTransform(sceneObjectID))
            {
                t->rot.x = x;
                t->rot.y = y;
            }
        }

        // ============= Scale =============

        ENGINE_API float __stdcall GetScaleX(uint32_t sceneObjectID)
        {
            auto* t = getTransform(sceneObjectID);
            return t ? t->scale.x : 1.0f;
        }

        ENGINE_API float __stdcall GetScaleY(uint32_t sceneObjectID)
        {
            auto* t = getTransform(sceneObjectID);
            return t ? t->scale.y : 1.0f;
        }

        ENGINE_API void __stdcall SetScale(uint32_t sceneObjectID, float x, float y)
        {
            if (auto* t = getTransform(sceneObjectID))
            {
                t->scale.x = x;
                t->scale.y = y;
            }
        }

        // Sets a node's position relative to its parent node.
        // Equivalent to child.worldPos = parent.worldPos + (x, y).
        ENGINE_API void __stdcall SetLocalPosition(uint32_t sceneObjectID, float x, float y)
        {
            if (!gEngine) return;
            Editor::Node* node = gEngine->getEnoEditor().getSceneTree().getNodeBySceneObjectID(sceneObjectID);
            if (!node) return;
            auto& em = gEngine->getEntityManager();
            if (!node->entity_.has<ECSComponents::TransformComponent>(em)) return;
            auto& t = node->entity_.get<ECSComponents::TransformComponent>(em);

            if (node->parent_ && node->parent_->entity_.has<ECSComponents::TransformComponent>(em))
            {
                auto& pt = node->parent_->entity_.get<ECSComponents::TransformComponent>(em);
                t.pos.x = pt.pos.x + x;
                t.pos.y = pt.pos.y + y;
            }
            else
            {
                t.pos.x = x;
                t.pos.y = y;
            }
        }
    }
}
