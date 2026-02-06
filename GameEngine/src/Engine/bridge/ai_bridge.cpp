#include "eno_bridge.hpp"
#include "../component/components/transform.hpp"
#include "../component/components/player_component.hpp"
#include "../component/components/ai_component.hpp"
#include "../component/components/physics_component.hpp"
#include "../ai/ai_manager.hpp"
#include "../ai/ai_events.hpp"
#include <cmath>

// gEngine is defined in eno_bridge.cpp - declare it here
extern Engine::EnoEngine* gEngine;

namespace Bridge
{
    // Helper function to get entity from sceneObjectID
    static ECS::Entity GetEntityFromSceneObjectID(uint32_t sceneObjectID, Editor::EnoSceneTree& sceneTree)
    {
        Editor::Node* node = sceneTree.getNodeBySceneObjectID(sceneObjectID);
        if (!node) {
            return ECS::Entity(UINT32_MAX); // Invalid entity
        }
        return node->entity_;
    }

    extern "C" {
        // Get the player entity's position
        ENGINE_API bool __stdcall GetPlayerPosition(float* outX, float* outY)
        {
            if (!gEngine) {
                std::cerr << "[AI Bridge] Engine not initialized!" << std::endl;
                return false;
            }

            auto& entMng = gEngine->getEntityManager();

            // Find player entity
            auto playerEntities = entMng.findEntity<ECSComponents::PlayerComponent>();
            if (playerEntities.empty()) {
                return false;
            }

            ECS::Entity playerEntity = playerEntities[0];

            // Get player transform
            if (!playerEntity.has<ECSComponents::TransformComponent>(entMng)) {
                return false;
            }

            auto& transform = playerEntity.get<ECSComponents::TransformComponent>(entMng);
            *outX = transform.pos.x;
            *outY = transform.pos.y;

            return true;
        }

        // Get entity's current position (using sceneObjectID from scripts)
        ENGINE_API bool __stdcall GetEntityPosition(uint32_t sceneObjectID, float* outX, float* outY)
        {
            if (!gEngine) {
                std::cerr << "[AI Bridge] Engine not initialized!" << std::endl;
                return false;
            }

            auto& entMng = gEngine->getEntityManager();
            auto& sceneTree = gEngine->getEnoEditor().getSceneTree();
            ECS::Entity entity = GetEntityFromSceneObjectID(sceneObjectID, sceneTree);

            if (entity.id() == UINT32_MAX || !entity.has<ECSComponents::TransformComponent>(entMng)) {
                return false;
            }

            auto& transform = entity.get<ECSComponents::TransformComponent>(entMng);
            *outX = transform.pos.x;
            *outY = transform.pos.y;

            return true;
        }

        // Set entity's velocity for movement
        ENGINE_API bool __stdcall SetEntityVelocity(uint32_t sceneObjectID, float vx, float vy)
        {
            if (!gEngine) {
                std::cerr << "[AI Bridge] Engine not initialized!" << std::endl;
                return false;
            }

            auto& entMng = gEngine->getEntityManager();
            auto& sceneTree = gEngine->getEnoEditor().getSceneTree();
            ECS::Entity entity = GetEntityFromSceneObjectID(sceneObjectID, sceneTree);

            if (entity.id() == UINT32_MAX || !entity.has<ECSComponents::PhysicsComponent>(entMng)) {
                return false;
            }

            auto& physics = entity.get<ECSComponents::PhysicsComponent>(entMng);
            physics.velocity = Math::eVec2<float>{ vx, vy };

            return true;
        }

        // Request pathfinding to target position
        ENGINE_API bool __stdcall RequestPathToPosition(uint32_t sceneObjectID, float targetX, float targetY, bool allowDiagonal)
        {
            if (!gEngine) {
                std::cerr << "[AI Bridge] Engine not initialized!" << std::endl;
                return false;
            }

            auto& entMng = gEngine->getEntityManager();
            auto& sceneTree = gEngine->getEnoEditor().getSceneTree();
            ECS::Entity entity = GetEntityFromSceneObjectID(sceneObjectID, sceneTree);

            if (entity.id() == UINT32_MAX) {
                return false;
            }

            if (!entity.has<ECSComponents::TransformComponent>(entMng)) {
                return false;
            }

            if (!entity.has<ECSComponents::AIComponent>(entMng)) {
                return false;
            }

            auto& transform = entity.get<ECSComponents::TransformComponent>(entMng);
            Math::eVec2<float> start = transform.pos;
            Math::eVec2<float> goal = Math::eVec2<float>{ targetX, targetY };

            // Request pathfinding through event bus (use ECS entity ID)
            auto& bus = gEngine->getEventBus();
            Math::eVec2<float> nextPos = start;
            AI::PathfindRequest request{ entity.id(), start, goal, allowDiagonal, nextPos };

            bus.publish(request);

            // Check if a valid next position was computed
            return nextPos != start;
        }

        // Get next waypoint from pathfinding
        ENGINE_API bool __stdcall GetNextWaypoint(uint32_t sceneObjectID, float* outX, float* outY)
        {
            if (!gEngine) {
                std::cerr << "[AI Bridge] Engine not initialized!" << std::endl;
                return false;
            }

            auto& entMng = gEngine->getEntityManager();
            auto& sceneTree = gEngine->getEnoEditor().getSceneTree();
            ECS::Entity entity = GetEntityFromSceneObjectID(sceneObjectID, sceneTree);

            if (entity.id() == UINT32_MAX) {
                return false;
            }

            if (!entity.has<ECSComponents::AIComponent>(entMng)) {
                return false;
            }

            if (!entity.has<ECSComponents::TransformComponent>(entMng)) {
                return false;
            }

            auto& aiMng = gEngine->getAIManager();
            auto& pathAgent = aiMng.getPathAgent(entity);

            if (!pathAgent.hasPath()) {
                return false;
            }

            auto& pathfinder = aiMng.getPathfinder();
            Math::eVec2<int> nextTile = pathAgent.getCurrentTile();
            Math::eVec2<float> nextWorld = pathfinder.tileToWorld(nextTile);

            *outX = nextWorld.x;
            *outY = nextWorld.y;

            return true;
        }

        // Calculate distance between two positions
        ENGINE_API float __stdcall CalculateDistance(float x1, float y1, float x2, float y2)
        {
            float dx = x2 - x1;
            float dy = y2 - y1;
            return std::sqrt(dx * dx + dy * dy);
        }

        // Move entity towards target position with speed
        ENGINE_API bool __stdcall MoveTowards(uint32_t sceneObjectID, float targetX, float targetY, float speed)
        {
            if (!gEngine) {
                std::cerr << "[AI Bridge] Engine not initialized!" << std::endl;
                return false;
            }

            auto& entMng = gEngine->getEntityManager();
            auto& sceneTree = gEngine->getEnoEditor().getSceneTree();
            ECS::Entity entity = GetEntityFromSceneObjectID(sceneObjectID, sceneTree);

            if (entity.id() == UINT32_MAX) {
                return false;
            }

            if (!entity.has<ECSComponents::TransformComponent>(entMng)) {
                return false;
            }

            if (!entity.has<ECSComponents::PhysicsComponent>(entMng)) {
                return false;
            }

            auto& transform = entity.get<ECSComponents::TransformComponent>(entMng);
            auto& physics = entity.get<ECSComponents::PhysicsComponent>(entMng);

            // Calculate direction
            float dx = targetX - transform.pos.x;
            float dy = targetY - transform.pos.y;
            float distance = std::sqrt(dx * dx + dy * dy);

            if (distance < 0.1f) {
                // Already at target
                physics.velocity = Math::eVec2<float>{ 0.0f, 0.0f };
                return true;
            }

            // Normalize and apply speed
            float vx = (dx / distance) * speed;
            float vy = (dy / distance) * speed;

            physics.velocity = Math::eVec2<float>{ vx, vy };

            return true;
        }

        // Check if entity is close to target position
        ENGINE_API bool __stdcall IsNearPosition(uint32_t sceneObjectID, float targetX, float targetY, float threshold)
        {
            if (!gEngine) {
                std::cerr << "[AI Bridge] Engine not initialized!" << std::endl;
                return false;
            }

            auto& entMng = gEngine->getEntityManager();
            auto& sceneTree = gEngine->getEnoEditor().getSceneTree();
            ECS::Entity entity = GetEntityFromSceneObjectID(sceneObjectID, sceneTree);

            if (entity.id() == UINT32_MAX) {
                return false;
            }

            if (!entity.has<ECSComponents::TransformComponent>(entMng)) {
                return false;
            }

            auto& transform = entity.get<ECSComponents::TransformComponent>(entMng);
            float dx = targetX - transform.pos.x;
            float dy = targetY - transform.pos.y;
            float distance = std::sqrt(dx * dx + dy * dy);

            return distance < threshold;
        }

        // Clear entity's pathfinding data
        ENGINE_API bool __stdcall ClearPath(uint32_t sceneObjectID)
        {
            if (!gEngine) {
                std::cerr << "[AI Bridge] Engine not initialized!" << std::endl;
                return false;
            }

            auto& entMng = gEngine->getEntityManager();
            auto& sceneTree = gEngine->getEnoEditor().getSceneTree();
            ECS::Entity entity = GetEntityFromSceneObjectID(sceneObjectID, sceneTree);

            if (entity.id() == UINT32_MAX) {
                return false;
            }

            if (!entity.has<ECSComponents::AIComponent>(entMng)) {
                return false;
            }

            auto& aiMng = gEngine->getAIManager();
            auto& pathAgent = aiMng.getPathAgent(entity);
            pathAgent.clear();

            return true;
        }

        // Get AI component parameters (using sceneObjectID from scripts)
        ENGINE_API bool __stdcall GetAIComponent(uint32_t sceneObjectID, float* outAwarenessRadius, float* outChaseSpeed,
            float* outAttackRange, float* outWindupTime, float* outAttackDuration, float* outHurtDuration,
            int* outHealth, bool* outAllowDiagonal)
        {
            if (!gEngine) {
                std::cerr << "[AI Bridge] Engine not initialized!" << std::endl;
                return false;
            }

            // Get node from sceneObjectID
            auto& sceneTree = gEngine->getEnoEditor().getSceneTree();

            Editor::Node* node = sceneTree.getNodeBySceneObjectID(sceneObjectID);

            if (!node) {
                std::cerr << "[AI Bridge] No node found for sceneObjectID: " << sceneObjectID << std::endl;
                return false;
            }

            auto& entMng = gEngine->getEntityManager();
            ECS::Entity entity = node->entity_;

            if (!entity.has<ECSComponents::AIComponent>(entMng)) {
                std::cerr << "[AI Bridge] Entity " << entity.id() << " (sceneObjectID: " << sceneObjectID << ") has no AIComponent!" << std::endl;
                return false;
            }

            auto& aiComp = entity.get<ECSComponents::AIComponent>(entMng);

            *outAwarenessRadius = aiComp.awarenessRadius;
            *outChaseSpeed = aiComp.chaseSpeed;
            *outAttackRange = aiComp.attackRange;
            *outWindupTime = aiComp.windupTime;
            *outAttackDuration = aiComp.attackDuration;
            *outHurtDuration = aiComp.hurtDuration;
            *outHealth = aiComp.health;
            *outAllowDiagonal = aiComp.allowDiagonal;

            return true;
        }

        // Set AI component health
        ENGINE_API bool __stdcall SetAIHealth(uint32_t sceneObjectID, int health)
        {
            if (!gEngine) {
                std::cerr << "[AI Bridge] Engine not initialized!" << std::endl;
                return false;
            }

            auto& entMng = gEngine->getEntityManager();
            auto& sceneTree = gEngine->getEnoEditor().getSceneTree();
            ECS::Entity entity = GetEntityFromSceneObjectID(sceneObjectID, sceneTree);

            if (entity.id() == UINT32_MAX) {
                return false;
            }

            if (!entity.has<ECSComponents::AIComponent>(entMng)) {
                return false;
            }

            auto& aiComp = entity.get<ECSComponents::AIComponent>(entMng);
            aiComp.health = health;

            return true;
        }

        // Set entity's position directly (teleport)
        ENGINE_API bool __stdcall SetEntityPosition(uint32_t sceneObjectID, float x, float y)
        {
            if (!gEngine) {
                std::cerr << "[AI Bridge] Engine not initialized!" << std::endl;
                return false;
            }

            auto& entMng = gEngine->getEntityManager();
            auto& sceneTree = gEngine->getEnoEditor().getSceneTree();
            ECS::Entity entity = GetEntityFromSceneObjectID(sceneObjectID, sceneTree);

            if (entity.id() == UINT32_MAX || !entity.has<ECSComponents::TransformComponent>(entMng)) {
                return false;
            }

            auto& transform = entity.get<ECSComponents::TransformComponent>(entMng);
            transform.pos.x = x;
            transform.pos.y = y;

            return true;
        }

        // Set the player entity's position directly (teleport) and zero velocity
        ENGINE_API bool __stdcall SetPlayerPosition(float x, float y)
        {
            if (!gEngine) {
                std::cerr << "[AI Bridge] Engine not initialized!" << std::endl;
                return false;
            }

            auto& entMng = gEngine->getEntityManager();
            auto playerEntities = entMng.findEntity<ECSComponents::PlayerComponent>();
            if (playerEntities.empty()) {
                return false;
            }

            ECS::Entity playerEntity = playerEntities[0];
            if (!playerEntity.has<ECSComponents::TransformComponent>(entMng)) {
                return false;
            }

            auto& transform = playerEntity.get<ECSComponents::TransformComponent>(entMng);
            transform.pos.x = x;
            transform.pos.y = y;

            // Zero velocity so physics doesn't move the player away from the destination
            if (playerEntity.has<ECSComponents::PhysicsComponent>(entMng)) {
                auto& physics = playerEntity.get<ECSComponents::PhysicsComponent>(entMng);
                physics.velocity = Math::eVec2<float>{ 0.0f, 0.0f };
                physics.ClearForces();
            }

            return true;
        }

        // Get the camera's world position (reads from engine's script camera)
        ENGINE_API bool __stdcall GetCameraPosition(float* outX, float* outY)
        {
            if (!gEngine) {
                std::cerr << "[AI Bridge] Engine not initialized!" << std::endl;
                return false;
            }

            auto pos = gEngine->getScriptCameraPos();
            *outX = pos.x;
            *outY = pos.y;
            return true;
        }

        // Set the camera's world position (writes to engine's script camera)
        ENGINE_API bool __stdcall SetCameraPosition(float x, float y)
        {
            if (!gEngine) {
                std::cerr << "[AI Bridge] Engine not initialized!" << std::endl;
                return false;
            }

            gEngine->setScriptCameraPos(x, y);
            return true;
        }
    }
}
