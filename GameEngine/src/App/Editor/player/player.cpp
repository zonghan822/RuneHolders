/*============================================================================================
* All content ? 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         player.cpp
* @author       Kimberly Charlotte Madlangbayan Ambrosio (k.ambrosio@digipen.edu)
* @date         05-11-2025
* @brief        FORCE-ONLY player movement system. Player movement is entirely force-based:
*               no direct velocity or position manipulation. The PhysicsSystem handles all
*               velocity integration, collision detection, and momentum transfer.
*===========================================================================================*/

#include "player.hpp"
#include <GLFW/glfw3.h>

namespace Player
{
    // HELPER FUNCTIONS

    /*============================================================================================
     * \brief       Manual power function for integer exponents
     * \param[in]   float base - Base value
     * \param[in]   int exponent - Integer exponent
     * \return      float - Result of base raised to exponent
     *===========================================================================================*/
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

    /*============================================================================================
     * \brief       Easing function for smooth dash using cubic easing
     * \param[in]   float x - Progress value (0.0 to 1.0)
     * \return      float - Eased value using formula: 1 - (1-x)³
     *===========================================================================================*/
    float EaseOutCubic(float x)
    {
        float oneMinusX = 1.0f - x;
        return 1.0f - Pow(oneMinusX, 3);
    }

    /*============================================================================================
     * \brief       PURE FORCE-BASED player update function
     * \param[in]   ECS::EntityManager& entityManager - Entity manager for accessing entities
     * \param[in]   Input::EnoInputManager& inputManager - Input system for WASD/Space controls
     * \param[in]   float deltaTime - Time elapsed since last frame (seconds)
     * \return      void
     *
     * NEW PHYSICS FLOW (Pure Force-Based):
     * ==================================
     * PlayerSystem (this file):
     *   1. Read WASD input
     *   2. Calculate movement force: F = direction * movementForce
     *   3. Apply force to PhysicsComponent: physics.AddForce(movementForce)
     *   4. Apply friction force: F_friction = -velocity * frictionCoeff * mass
     *   5. [Optional] Apply jump force, dash force, etc.
     *
     * PhysicsSystem (called after PlayerSystem):
     *   6. Integrate forces: acceleration = F_total / mass
     *   7. Integrate velocity: velocity += acceleration * dt
     *   8. Apply velocity damping (drag)
     *   9. Clamp velocity to max speed
     *   10. Integrate position: position += velocity * dt
     *   11. Detect collisions using AABB
     *   12. Resolve collisions with momentum transfer (enables pushing!)
     *   13. Clear forces for next frame
     *
     * CRITICAL: PlayerSystem NO LONGER touches velocity or position directly!
     * All movement is handled through forces accumulated in PhysicsComponent.force
     *===========================================================================================*/
    void PlayerSystem::Update(ECS::EntityManager& entityManager, Input::EnoInputManager& inputManager, float deltaTime)
    {
        // Get the entity map which contains all entities with their locations
        auto& entityMap = entityManager.getMap();

        // Iterate through ALL entities in the world
        for (auto& [entityID, entityLoc] : entityMap)
        {
            // Check if this entity has BOTH PlayerComponent AND PhysicsComponent
            bool hasPlayer = false;
            bool hasPhysics = false;

            for (const auto& type : entityLoc.archetype->types_)
            {
                if (type == std::type_index(typeid(ECSComponents::PlayerComponent)))
                    hasPlayer = true;
                else if (type == std::type_index(typeid(ECSComponents::PhysicsComponent)))
                    hasPhysics = true;
            }

            // Skip entities that don't have both components
            if (!hasPlayer || !hasPhysics)
                continue;

            // Get the player and physics components for this entity
            size_t playerIndex = entityLoc.archetype->findComponentIndex<ECSComponents::PlayerComponent>();
            auto& playerComp = entityLoc.archetype->chunks_[entityLoc.chunkIndex]->retrieve<ECSComponents::PlayerComponent>(
                playerIndex, entityLoc.entityIndexInChunk);

            size_t physicsIndex = entityLoc.archetype->findComponentIndex<ECSComponents::PhysicsComponent>();
            auto& physicsComp = entityLoc.archetype->chunks_[entityLoc.chunkIndex]->retrieve<ECSComponents::PhysicsComponent>(
                physicsIndex, entityLoc.entityIndexInChunk);

            // Skip if player cannot move (e.g., stunned, frozen)
            if (!playerComp.canMove)
                continue;

            // Get current transform for dash mechanics
            auto& transform = GetTransform(entityLoc);

            // ====== UPDATE DASH COOLDOWN ======
            if (playerComp.dashCooldownTimer > 0.0f)
            {
                playerComp.dashCooldownTimer -= deltaTime;
                if (playerComp.dashCooldownTimer < 0.0f)
                    playerComp.dashCooldownTimer = 0.0f;
            }

            // ====== DASH LOGIC (instant burst) ======
            if (playerComp.isDashing)
            {
                // Update dash timer
                playerComp.dashTimer += deltaTime;     

                if (playerComp.dashTimer >= playerComp.dashDuration)
                {
                    // Dash complete - immediately stop dashing
                    playerComp.isDashing = false;
                    playerComp.dashTimer = 0.0f;
                    playerComp.dashCooldownTimer = playerComp.dashCooldown;

                     // Get current input for immediate transition to normal movement
                    Math::eVec2<float> currentInput = GetMovementInput(inputManager);

                    if (Math::eVec2SquaredLength(currentInput) > 0.01f)
                    {
                        // Return to normal movement speed instantly
                        physicsComp.velocity.x = currentInput.x * playerComp.maxSpeed;
                        physicsComp.velocity.y = currentInput.y * playerComp.maxSpeed;
                    }
                    else 
                    {
                        // No input - stop instantly
                        physicsComp.velocity.x = 0.0f;
                        physicsComp.velocity.y = 0.0f;
                    }
                }
                else
                {
                    
                    float dashSpeed = playerComp.dashDistance / playerComp.dashDuration;

                    // Set velocity to constant dash speed in dash direction
                    physicsComp.velocity.x = playerComp.dashDirection.x * dashSpeed;
                    physicsComp.velocity.y = playerComp.dashDirection.y * dashSpeed;
                }

                // During dash, skip normal movement input
                continue;
            }

            // ====== INITIATE DASH WITH SPACEBAR KEY ======
            if (inputManager.isKeyPressed(GLFW_KEY_SPACE) && playerComp.dashCooldownTimer <= 0.0f && playerComp.canMove)
            {
                // Get current movement input to determine dash direction
                Math::eVec2<float> input = GetMovementInput(inputManager);

                // If no input, dash in last movement direction (use velocity direction)
                if (Math::eVec2SquaredLength(input) < 0.01f)
                {
                    // Use current velocity direction, or default to down
                    float velLenSq = Math::eVec2SquaredLength(physicsComp.velocity);
                    if (velLenSq > 0.01f)
                    {
                        Math::eVec2Normalize(input, physicsComp.velocity);
                    }
                    else
                    {
                        input = { 0.0f, 1.0f };  // Default dash down
                    }
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

            // ====== FORCE-BASED MOVEMENT (Pure Physics) ======
            // NOTE: We do NOT clear forces here because PhysicsSystem will do it
            // This allows multiple systems to add forces in the same frame

            Math::eVec2<float> input = GetMovementInput(inputManager);

            // INSTANT STOP: Zero velocity immediately when no input
            if (Math::eVec2SquaredLength(input) < 0.01f)
            {
                physicsComp.velocity.x = 0.0f;
                physicsComp.velocity.y = 0.0f;
            }
            else
            {
                // CONSTANT SPEED: Set velocity directly to max speed * direction
                // No acceleration - instant full speed
                Math::eVec2<float> targetVelocity = {
                    input.x * playerComp.maxSpeed,
                    input.y * playerComp.maxSpeed
                };

                physicsComp.velocity.x = targetVelocity.x;
                physicsComp.velocity.y = targetVelocity.y;
            }
        }
    }

    /*============================================================================================
     * \brief       Get normalized WASD movement input from keyboard
     * \param[in]   Input::EnoInputManager& inputManager - Input system reference
     * \return      Math::eVec2<float> - Normalized movement direction (-1 to 1 on each axis)
     *===========================================================================================*/
    Math::eVec2<float> PlayerSystem::GetMovementInput(Input::EnoInputManager& inputManager)
    {
        Math::eVec2<float> input = { 0.0f, 0.0f };

        // Use EnoInputManager's isKeyPressed() for continuous WASD input
        if (inputManager.isKeyPressed(GLFW_KEY_A))  // Move left
            input.x -= 1.0f;
        if (inputManager.isKeyPressed(GLFW_KEY_D))  // Move right
            input.x += 1.0f;
        if (inputManager.isKeyPressed(GLFW_KEY_W))  // Move up
            input.y -= 1.0f;
        if (inputManager.isKeyPressed(GLFW_KEY_S))  // Move down
            input.y += 1.0f;

        // Normalize diagonal movement to prevent faster diagonal speed
        float lengthSquared = Math::eVec2SquaredLength(input);
        if (lengthSquared > 1.0f)
        {
            float length = sqrtf(lengthSquared);
            input.x /= length;
            input.y /= length;
        }

        return input;
    }

    /*============================================================================================
     * \brief       DEPRECATED - No longer needed with PhysicsSystem
     * \detail      Collision checking is now handled by PhysicsSystem during velocity integration
     *===========================================================================================*/
    bool PlayerSystem::CanMoveTo(
        [[maybe_unused]] const ECS::EntityLoc& playerEntity,
        [[maybe_unused]] const ECSComponents::CollisionComponent& playerCollider,
        [[maybe_unused]] const Math::eVec2<float>& targetPos,
        [[maybe_unused]] ECS::EntityManager& entityManager)
    {
        // This function is kept for backward compatibility but is no longer used
        // PhysicsSystem handles all collision detection now
        return true;
    }

    /*============================================================================================
     * \brief       Helper functions to get components from entity location
     *===========================================================================================*/
    ECSComponents::TransformComponent& PlayerSystem::GetTransform(const ECS::EntityLoc& entityLoc)
    {
        size_t index = entityLoc.archetype->findComponentIndex<ECSComponents::TransformComponent>();
        return entityLoc.archetype->chunks_[entityLoc.chunkIndex]->retrieve<ECSComponents::TransformComponent>(
            index, entityLoc.entityIndexInChunk);
    }

    ECSComponents::CollisionComponent& PlayerSystem::GetCollision(const ECS::EntityLoc& entityLoc)
    {
        size_t index = entityLoc.archetype->findComponentIndex<ECSComponents::CollisionComponent>();
        return entityLoc.archetype->chunks_[entityLoc.chunkIndex]->retrieve<ECSComponents::CollisionComponent>(
            index, entityLoc.entityIndexInChunk);
    }

} // namespace Player