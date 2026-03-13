/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         eno_bridge_physics.cpp
* @author       Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   67%
* @co-author    ZongHan Law (zonghan.law@digipen.edu)   33%
* @brief        Physics component bridge — velocity, forces, mass, gravity, kinematic.
*//*========================================================================================*/



#include "eno_bridge.hpp"
#include "../component/components/physics_component.hpp"

namespace Bridge
{
    static ECSComponents::PhysicsComponent* getPhysics(uint32_t sceneObjectID)
    {
        if (!gEngine) return nullptr;
        Editor::Node* node = gEngine->getEnoEditor().getSceneTree().getNodeBySceneObjectID(sceneObjectID);
        if (!node) return nullptr;
        if (!node->entity_.has<ECSComponents::PhysicsComponent>(gEngine->getEntityManager())) return nullptr;
        return &node->entity_.get<ECSComponents::PhysicsComponent>(gEngine->getEntityManager());
    }

    extern "C"
    {
        // ============= Velocity =============

        ENGINE_API float __stdcall GetVelocityX(uint32_t sceneObjectID)
        {
            auto* p = getPhysics(sceneObjectID);
            return p ? p->velocity.x : 0.0f;
        }

        ENGINE_API float __stdcall GetVelocityY(uint32_t sceneObjectID)
        {
            auto* p = getPhysics(sceneObjectID);
            return p ? p->velocity.y : 0.0f;
        }

        ENGINE_API void __stdcall SetVelocity(uint32_t sceneObjectID, float x, float y)
        {
            if (auto* p = getPhysics(sceneObjectID))
            {
                p->velocity.x = x;
                p->velocity.y = y;
            }
        }

        // ============= Forces =============

        ENGINE_API void __stdcall AddForce(uint32_t sceneObjectID, float x, float y)
        {
            if (auto* p = getPhysics(sceneObjectID))
                p->AddForce({ x, y });
        }

        // ============= Mass =============

        ENGINE_API float __stdcall GetMass(uint32_t sceneObjectID)
        {
            auto* p = getPhysics(sceneObjectID);
            return p ? p->mass : 1.0f;
        }

        ENGINE_API void __stdcall SetMass(uint32_t sceneObjectID, float mass)
        {
            if (auto* p = getPhysics(sceneObjectID))
                p->mass = mass;
        }

        // ============= Gravity =============

        ENGINE_API bool __stdcall GetUseGravity(uint32_t sceneObjectID)
        {
            auto* p = getPhysics(sceneObjectID);
            return p ? p->useGravity : false;
        }

        ENGINE_API void __stdcall SetUseGravity(uint32_t sceneObjectID, bool useGravity)
        {
            if (auto* p = getPhysics(sceneObjectID))
                p->useGravity = useGravity;
        }

        // ============= Kinematic =============

        ENGINE_API bool __stdcall GetIsKinematic(uint32_t sceneObjectID)
        {
            auto* p = getPhysics(sceneObjectID);
            return p ? p->isKinematic : false;
        }

        ENGINE_API void __stdcall SetIsKinematic(uint32_t sceneObjectID, bool kinematic)
        {
            if (auto* p = getPhysics(sceneObjectID))
                p->isKinematic = kinematic;
        }
    }
}
