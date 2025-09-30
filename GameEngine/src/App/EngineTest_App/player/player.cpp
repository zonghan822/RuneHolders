/*************************************************
* @file player.cpp
* @brief Implementation of player system
* @author Your Team
**************************************************/

#include "player.hpp"

namespace Player
{
    //======================================================================================
    // PLAYER SYSTEM IMPLEMENTATION
    //======================================================================================

    void PlayerSystem::Update(ECS::EntityManager& entityManager, GLFWwindow* window, float deltaTime)
    {
        // Find all player entities
        auto& entityMap = entityManager.getMap();

        for (auto& [entityID, entityLoc] : entityMap)
        {
            // Check if this entity is a player with required components
            bool hasPlayer = false;
            bool hasTransform = false;
            bool hasCollision = false;

            for (const auto& type : entityLoc.archetype->types_)
            {
                if (type == std::type_index(typeid(PlayerComponent)))
                    hasPlayer = true;
                else if (type == std::type_index(typeid(ECSComponents::TransformComponent)))
                    hasTransform = true;
                else if (type == std::type_index(typeid(Collision::CollisionComponent)))
                    hasCollision = true;
            }

            // Skip if not a player or missing components
            if (!hasPlayer || !hasTransform || !hasCollision)
                continue;

            // Get components
            auto& transform = GetTransform(entityLoc);
            auto& collision = GetCollision(entityLoc);

            // Get player component
            size_t playerIndex = entityLoc.archetype->findComponentIndex<PlayerComponent>();
            auto& player = entityLoc.archetype->chunks_[entityLoc.chunkIndex]->retrieve<PlayerComponent>(
                playerIndex, entityLoc.entityIndexInChunk);

            // Skip if player can't move
            if (!player.canMove)
                continue;

            // Get input direction
            Math::eVec2<float> inputDir = GetMovementInput(window);

            // Calculate movement delta
            Math::eVec2<float> moveDelta = {
                inputDir.v1 * player.moveSpeed * deltaTime,
                inputDir.v2 * player.moveSpeed * deltaTime
            };

            // Calculate target position
            Math::eVec2<float> targetPos = {
                transform.pos.x + moveDelta.v1,
                transform.pos.y + moveDelta.v2
            };

            // Check if we can move there (collision test)
            if (CanMoveTo(entityLoc, collision, targetPos, entityManager))
            {
                // No collision - move to target position
                transform.pos.x = targetPos.v1;
                transform.pos.y = targetPos.v2;
            }
            else {
                // collision detected - try sliding along walls
                // try X movement only
                Math::eVec2<float> xOnlyPos = { targetPos.v1, transform.pos.y };
                if (CanMoveTo(entityLoc, collision, xOnlyPos, entityManager))
                {
                    transform.pos.x = xOnlyPos.v1;
                }

                // try Y movement only
                Math::eVec2<float> yOnlyPos = { transform.pos.x, targetPos.v2 };
                if (CanMoveTo(entityLoc, collision, yOnlyPos, entityManager))
                {
                    transform.pos.y = yOnlyPos.v2;
                }
            }
        }
    }

    Math::eVec2<float> PlayerSystem::GetMovementInput(GLFWwindow* window)
    {
        Math::eVec2<float> input = { 0.0f, 0.0f };

        // Read WASD keys
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            input.v2 -= 1.0f;  // Up
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            input.v2 += 1.0f;  // Down
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            input.v1 -= 1.0f;  // Left
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            input.v1 += 1.0f;  // Right

        // Normalize diagonal movement using engine's math library
        float lengthSquared = Math::eVec2SquaredLength(input);
        if (lengthSquared > 0.0f)
        {
            // Use your math library's normalize function
            Math::eVec2<float> normalized;
            Math::eVec2Normalize(normalized, input);
            input = normalized;
        }

        return input;
    }

    bool PlayerSystem::CanMoveTo(
        const ECS::EntityLoc& playerEntity,
        const Collision::CollisionComponent& playerCollider,
        const Math::eVec2<float>& targetPos,
        ECS::EntityManager& entityManager)
    {
        // Create temporary transform at target position for collision testing
        ECSComponents::TransformComponent testTransform;
        testTransform.pos.x = targetPos.v1;
        testTransform.pos.y = targetPos.v2;
        testTransform.pos.z = 0.0f;
        testTransform.rot = { 0.0f, 0.0f, 0.0f };
        testTransform.scale = { 1.0f, 1.0f, 1.0f };

        // Check collision with all other entities
        auto& entityMap = entityManager.getMap();

        for (auto& [otherID, otherLoc] : entityMap)
        {
            // Don't check collision with self
            if (&otherLoc == &playerEntity)
                continue;

            // Check if other entity has transform and collision
            bool hasTransform = false;
            bool hasCollision = false;

            for (const auto& type : otherLoc.archetype->types_)
            {
                if (type == std::type_index(typeid(ECSComponents::TransformComponent)))
                    hasTransform = true;
                else if (type == std::type_index(typeid(Collision::CollisionComponent)))
                    hasCollision = true;
            }

            if (!hasTransform || !hasCollision)
                continue;

            // Get other entity's components
            auto& otherTransform = GetTransform(otherLoc);
            auto& otherCollider = GetCollision(otherLoc);

            // Skip inactive colliders and triggers
            if (!otherCollider.isActive || otherCollider.isTrigger)
                continue;

            // DEBUG: Print positions before collision check
            std::cout << "[COLLISION CHECK]" << std::endl;
            std::cout << "  Player target pos: (" << targetPos.v1 << ", " << targetPos.v2 << ")" << std::endl;
            std::cout << "  Player collider: " << playerCollider.data.rectangle.width << "x" << playerCollider.data.rectangle.height << std::endl;
            std::cout << "  Entity " << otherID << " pos: (" << otherTransform.pos.x << ", " << otherTransform.pos.y << ")" << std::endl;
            std::cout << "  Entity collider: " << otherCollider.data.rectangle.width << "x" << otherCollider.data.rectangle.height << std::endl;

            // Perform collision test at target position
            Collision::CollisionResult result = Collision::CollisionDetection::CheckCollision(
                playerCollider, testTransform,
                otherCollider, otherTransform
            );



            // If collision detected, can't move there
            if (result.hasCollision) {
                std::cout << "  >>> COLLISION DETECTED! <<<" << std::endl;
                std::cout << "  Penetration depth: " << result.penetrationDepth << std::endl;
                std::cout << "  Penetration vector: (" << result.penetrationVector.v1 << ", " << result.penetrationVector.v2 << ")" << std::endl;
                std::cout << "  Contact point: (" << result.contactPoint.v1 << ", " << result.contactPoint.v2 << ")" << std::endl;
                std::cout << std::endl;
                return false;
            }
            else
            {
                std::cout << "  No collision" << std::endl;
                std::cout << std::endl;
            }
               
        }

        // No collisions - movement is valid
        return true;
    }

    ECSComponents::TransformComponent& PlayerSystem::GetTransform(const ECS::EntityLoc& entityLoc)
    {
        size_t index = entityLoc.archetype->findComponentIndex<ECSComponents::TransformComponent>();
        return entityLoc.archetype->chunks_[entityLoc.chunkIndex]->retrieve<ECSComponents::TransformComponent>(
            index, entityLoc.entityIndexInChunk);
    }

    Collision::CollisionComponent& PlayerSystem::GetCollision(const ECS::EntityLoc& entityLoc)
    {
        size_t index = entityLoc.archetype->findComponentIndex<Collision::CollisionComponent>();
        return entityLoc.archetype->chunks_[entityLoc.chunkIndex]->retrieve<Collision::CollisionComponent>(
            index, entityLoc.entityIndexInChunk);
    }

} // namespace Player