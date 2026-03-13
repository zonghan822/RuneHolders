/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         eno_bridge_collision.cpp
* @author       ZongHan Law (zonghan.law@digipen.edu)
* @brief        Collision bridge — point overlap, entity-entity collision, collider activation. All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
*//*========================================================================================*/

#include "eno_bridge.hpp"
#include "../component/components/collision_component.hpp"
#include "../component/components/transform.hpp"
#include "../collision/collision.hpp"

namespace Bridge
{
    static Editor::Node* getNode(uint32_t sceneObjectID)
    {
        if (!gEngine) return nullptr;
        return gEngine->getEnoEditor().getSceneTree().getNodeBySceneObjectID(sceneObjectID);
    }

    static ECSComponents::CollisionComponent* getCollider(uint32_t sceneObjectID)
    {
        Editor::Node* node = getNode(sceneObjectID);
        if (!node) return nullptr;
        if (!node->entity_.has<ECSComponents::CollisionComponent>(gEngine->getEntityManager())) return nullptr;
        return &node->entity_.get<ECSComponents::CollisionComponent>(gEngine->getEntityManager());
    }

    static ECSComponents::TransformComponent* getTransform(uint32_t sceneObjectID)
    {
        Editor::Node* node = getNode(sceneObjectID);
        if (!node) return nullptr;
        if (!node->entity_.has<ECSComponents::TransformComponent>(gEngine->getEntityManager())) return nullptr;
        return &node->entity_.get<ECSComponents::TransformComponent>(gEngine->getEntityManager());
    }

    extern "C"
    {
        // Returns true if the entity's collider overlaps the world-space point (x, y).
        // Useful for mouse-click hit testing on runes/buttons.
        ENGINE_API bool __stdcall IsOverlappingPoint(uint32_t sceneObjectID, float x, float y)
        {
            auto* collider  = getCollider(sceneObjectID);
            auto* transform = getTransform(sceneObjectID);
            if (!collider || !transform || !collider->isActive) return false;

            ECSComponents::CollisionComponent pointCol;
            pointCol.type     = ECSComponents::ColliderType::POINT;
            pointCol.isActive = true;
            pointCol.isTrigger = true;

            ECSComponents::TransformComponent pointTransform;
            pointTransform.pos = { x, y };

            Collision::CollisionResult result = Collision::CollisionDetection::CheckCollision(
                pointCol,  pointTransform,
                *collider, *transform);

            return result.hasCollision;
        }

        // Returns true if the two entities' colliders are currently overlapping.
        ENGINE_API bool __stdcall IsColliding(uint32_t sceneObjectIDA, uint32_t sceneObjectIDB)
        {
            auto* colliderA  = getCollider(sceneObjectIDA);
            auto* transformA = getTransform(sceneObjectIDA);
            auto* colliderB  = getCollider(sceneObjectIDB);
            auto* transformB = getTransform(sceneObjectIDB);

            if (!colliderA || !transformA || !colliderA->isActive) return false;
            if (!colliderB || !transformB || !colliderB->isActive) return false;

            Collision::CollisionResult result = Collision::CollisionDetection::CheckCollision(
                *colliderA, *transformA,
                *colliderB, *transformB);

            return result.hasCollision;
        }

        ENGINE_API bool __stdcall GetColliderActive(uint32_t sceneObjectID)
        {
            auto* c = getCollider(sceneObjectID);
            return c ? c->isActive : false;
        }

        ENGINE_API void __stdcall SetColliderActive(uint32_t sceneObjectID, bool active)
        {
            if (auto* c = getCollider(sceneObjectID))
                c->isActive = active;
        }
    }
}
