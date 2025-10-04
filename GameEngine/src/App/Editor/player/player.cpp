/*============================================================================================
* @file         player.cpp
* @author       Kimberly Charlotte Madlangbayan Ambrosio (k.ambrosio@digipen.edu)
* @date         03-10-2025
* @brief        Implementation of player system including movement logic with collision
*               detection, dash mechanics with easing, and helper functions for component
*               retrieval from the ECS architecture.
*//*========================================================================================*/

#include "player.hpp"
#include <GLFW/glfw3.h>

namespace Player
{

    // =============================================================================
    // HELPER FUNCTIONS
    // =============================================================================
    
    // Manual power function for integer exponents
    float Pow(float base, int exponent)
    {
        if (exponent == 0) return 1.0f;

        float result = 1.0f;
        int absExp = (exponent < 0) ? -exponent : exponent;

        for (int i = 0; i < absExp; ++i)
        {
            result *= base;
        }

        return (exponent < 0) ? (1.0f / result) : result;
    }

    // Easing function for smooth dash deceleration
    float EaseOutCubic(float x) {
        
        float oneMinusX = 1.0f - x;
        return 1.0f - Pow(oneMinusX, 3);
    }

    //======================================================================================
    // PLAYER SYSTEM IMPLEMENTATION
    //======================================================================================
    void PlayerSystem::Update(ECS::EntityManager& entityManager, Input::EnoInputManager& inputManager, float deltaTime)
    {
        // Get the entity map which contains all entities with their locations
        auto& entityMap = entityManager.getMap();

        // Iterate through ALL entities in the world
        for (auto& [entityID, entityLoc] : entityMap)
        {
            // Check if this entity has a PlayerComponent
            // We need to check if the archetype contains the Player::PlayerComponent type
            bool hasPlayer = false;
            for (const auto& type : entityLoc.archetype->types_)
            {
                if (type == std::type_index(typeid(Player::PlayerComponent)))
                {
                    hasPlayer = true;
                    break;
                }
            }

            // Skip entities that don't have a PlayerComponent
            if (!hasPlayer)
                continue;

            // Get the player component for this entity
            size_t playerIndex = entityLoc.archetype->findComponentIndex<Player::PlayerComponent>();
            auto& playerComp = entityLoc.archetype->chunks_[entityLoc.chunkIndex]->retrieve<Player::PlayerComponent>(
                playerIndex, entityLoc.entityIndexInChunk);

            // Skip if player cannot move (e.g., stunned, frozen)
            if (!playerComp.canMove)
                continue;

            // Get current transform and collision components using EntityLoc
            auto& transform = GetTransform(entityLoc);
            auto& collider = GetCollision(entityLoc);

            // Update Dash Cooldown
            if (playerComp.dashCooldownTimer > 0.0f)
            {
                playerComp.dashCooldownTimer -= deltaTime;
                if (playerComp.dashCooldownTimer < 0.0f)
                    playerComp.dashCooldownTimer = 0.0f;
            }

            // Dash logic
            if (playerComp.isDashing)
            {
                // Update dash timer
                playerComp.dashTimer += deltaTime;

                // Calculate progress (0 to 1)
                float progress = playerComp.dashTimer / playerComp.dashDuration;

                if (progress >= 1.0f)
                {
                    // Dash complete
                    progress = 1.0f;
                    playerComp.isDashing = false;
                    playerComp.dashTimer = 0.0f;
                    playerComp.dashCooldownTimer = playerComp.dashCooldown;
                }

                // Apply easing function
                float easedProgress = EaseOutCubic(progress);

                // Calculate target position
                Math::eVec2<float> targetPos =
                {
                    playerComp.dashStartPos.v1 + (playerComp.dashDirection.v1 * playerComp.dashDistance * easedProgress),
                    playerComp.dashStartPos.v2 + (playerComp.dashDirection.v2 * playerComp.dashDistance * easedProgress)
                };

                // Check if we can move to target position (collision check)
                if (CanMoveTo(entityLoc, collider, targetPos, entityManager))
                {
                    transform.pos.x = targetPos.v1;
                    transform.pos.y = targetPos.v2;
                }
                else {
                    // Hit something - end dash early
                    playerComp.isDashing = false;
                    playerComp.dashTimer = 0.0f;
                    playerComp.dashCooldownTimer = playerComp.dashCooldown;
                }

                // Skip normal movement during dash
                continue;
            }

            // Initiate Dash with Spacebar Key
            if (inputManager.isKeyPressed(GLFW_KEY_SPACE) && playerComp.dashCooldownTimer <= 0.0f && playerComp.canMove)
            {
                // Get current movement input to determine dash direction
                Math::eVec2<float> input = GetMovementInput(inputManager);

                // If no input, dash in last faced direction (or default forward)
                if (Math::eVec2SquaredLength(input) < 0.01f)
                {
                    // Default dash forward (down in screen space)
                    input = { 0.0f, 1.0f };
                }

                // Normalize the direction
                Math::eVec2<float> dashDir;
                Math::eVec2Normalize(dashDir, input);

                // Start the dash
                playerComp.isDashing = true;
                playerComp.dashTimer = 0.0f;
                playerComp.dashDirection = dashDir;
                playerComp.dashStartPos = { transform.pos.x, transform.pos.y };

                // Skip normal movement this frame
                continue;
            }

            // ====== NORMAL MOVEMENT (when not dashing) ======
            if (!playerComp.canMove) continue;

            // Get input from EnoInputManager (WASD keys)
            Math::eVec2<float> input = GetMovementInput(inputManager);

            // Calculate desired movement based on input and move speed
            // deltaTime ensures smooth movement regardless of framerate
            Math::eVec2<float> movement = {
                input.v1 * playerComp.moveSpeed * deltaTime,
                input.v2 * playerComp.moveSpeed * deltaTime
            };

            // Calculate target position after movement
            Math::eVec2<float> targetPos = {
                transform.pos.x + movement.v1,
                transform.pos.y + movement.v2
            };

            // Check if the target position is valid (no collision with solid objects)
            if (CanMoveTo(entityLoc, collider, targetPos, entityManager))
            {
                // Movement is valid - update transform position
                transform.pos.x = targetPos.v1;
                transform.pos.y = targetPos.v2;
            }
            // If collision detected, player stays at current position (blocked)
        }
    }

    
     
    // Updated GetMovementInput to use EnoInputManager reference
    Math::eVec2<float> PlayerSystem::GetMovementInput(Input::EnoInputManager& inputManager)
    {
        Math::eVec2<float> input = { 0.0f, 0.0f };

        // Use EnoInputManager's isKeyPressed() for continuous WASD input
        // isKeyPressed() returns true while the key is held down
        if (inputManager.isKeyPressed(GLFW_KEY_W))
            input.v2 -= 1.0f;                       // Move up (negative Y in screen space)
        if (inputManager.isKeyPressed(GLFW_KEY_S))
            input.v2 += 1.0f;                       // Move down (positive Y in screen space)
        if (inputManager.isKeyPressed(GLFW_KEY_A))
            input.v1 -= 1.0f;                       // Move left (negative X)
        if (inputManager.isKeyPressed(GLFW_KEY_D))
            input.v1 += 1.0f;                       // Move right (positive X)

        // Normalize diagonal movement to ensure consistent speed
        float lengthSquared = Math::eVec2SquaredLength(input);
        if (lengthSquared > 0.0f)
        {
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
        extern float scaleSize;
        extern float rotationRadians;

        // Create temporary transform at target position for collision testing
        ECSComponents::TransformComponent testTransform;
        testTransform.pos.x = targetPos.v1;
        testTransform.pos.y = targetPos.v2;
        testTransform.pos.z = 0.0f;
        testTransform.rot = { 0.0f, 0.0f, 0.0f };   // apply rotation
        testTransform.scale = { scaleSize, scaleSize, 1.0f }; // apply scale

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
            //std::cout << "[COLLISION CHECK]" << std::endl;
            //std::cout << "  Player target pos: (" << targetPos.v1 << ", " << targetPos.v2 << ")" << std::endl;
            //std::cout << "  Player collider: " << playerCollider.data.rectangle.width << "x" << playerCollider.data.rectangle.height << std::endl;
            //std::cout << "  Entity " << otherID << " pos: (" << otherTransform.pos.x << ", " << otherTransform.pos.y << ")" << std::endl;
            //std::cout << "  Entity collider: " << otherCollider.data.rectangle.width << "x" << otherCollider.data.rectangle.height << std::endl;

            // Perform collision test at target position
            Collision::CollisionResult result = Collision::CollisionDetection::CheckCollision(
                playerCollider, testTransform,
                otherCollider, otherTransform
            );



            // If collision detected, can't move there
            if (result.hasCollision) {
                return false;
            }              
        }

        // No collisions - movement is valid
        return true;
    }

    // GET TRANSFORM - Helper to retrieve Transform component from EntityLoc
    ECSComponents::TransformComponent& PlayerSystem::GetTransform(const ECS::EntityLoc& entityLoc)
    {
        // Find the Transform component's index in this entity's archetype
        size_t index = entityLoc.archetype->findComponentIndex<ECSComponents::TransformComponent>();

        // Retrieve and return the component from the appropriate chunk
        return entityLoc.archetype->chunks_[entityLoc.chunkIndex]->retrieve<ECSComponents::TransformComponent>(
            index, entityLoc.entityIndexInChunk);
    }

    // GET COLLISION - Helper to retrieve Collision component from EntityLoc
    Collision::CollisionComponent& PlayerSystem::GetCollision(const ECS::EntityLoc& entityLoc)
    {
        // Find the Collision component's index in this entity's archetype
        size_t index = entityLoc.archetype->findComponentIndex<Collision::CollisionComponent>();

        // Retrieve and return the component from the appropriate chunk
        return entityLoc.archetype->chunks_[entityLoc.chunkIndex]->retrieve<Collision::CollisionComponent>(
            index, entityLoc.entityIndexInChunk);
    }

} // namespace Player