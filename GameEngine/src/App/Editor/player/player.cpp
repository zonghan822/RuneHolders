/*============================================================================================
* @file         player.cpp
* @author       Kimberly Charlotte Madlangbayan Ambrosio (k.ambrosio@digipen.edu)
* @date         03-11-2025
* @brief        Implementation of force-based player physics system. Uses proper Newtonian
*               physics: Force ? Acceleration ? Velocity ? Position. Works with both
*               PlayerComponent and PhysicsComponent for realistic movement.
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
     * \brief       Easing function for smooth dash deceleration using cubic easing
     * \param[in]   float x - Progress value (0.0 to 1.0)
     * \return      float - Eased value using formula: 1 - (1-x)³
     *===========================================================================================*/
    float EaseOutCubic(float x)
    {
        float oneMinusX = 1.0f - x;
        return 1.0f - Pow(oneMinusX, 3);
    }

    /*============================================================================================
     * \brief       Main update function implementing force-based physics movement
     * \param[in]   ECS::EntityManager& entityManager - Entity manager for accessing entities
     * \param[in]   Input::EnoInputManager& inputManager - Input system for WASD/Space controls
     * \param[in]   float deltaTime - Time elapsed since last frame (seconds)
     * \return      void
     *
     * Physics Implementation:
     * 1. Apply input forces to PhysicsComponent.force
     * 2. Apply friction as opposing force
     * 3. PhysicsComponent auto-calculates: acceleration = force / mass
     * 4. Integrate: velocity += acceleration * dt
     * 5. Clamp velocity to maxSpeed
     * 6. Integrate: position += velocity * dt (with collision)
     * 7. Clear forces for next frame
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

            // Get current transform and collision components using EntityLoc
            auto& transform = GetTransform(entityLoc);
            auto& collider = GetCollision(entityLoc);

            // ====== UPDATE DASH COOLDOWN ======
            if (playerComp.dashCooldownTimer > 0.0f)
            {
                playerComp.dashCooldownTimer -= deltaTime;
                if (playerComp.dashCooldownTimer < 0.0f)
                    playerComp.dashCooldownTimer = 0.0f;
            }

            // ====== DASH LOGIC (kinematic movement, ignores physics) ======
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

                    // Reset velocity after dash ends (prevents momentum carry-over)
                    physicsComp.velocity = { 0.0f, 0.0f };
                }

                // Apply easing function
                float easedProgress = EaseOutCubic(progress);

                // Calculate target position
                Math::eVec2<float> targetPos =
                {
                    playerComp.dashStartPos.x + (playerComp.dashDirection.x * playerComp.dashDistance * easedProgress),
                    playerComp.dashStartPos.y + (playerComp.dashDirection.y * playerComp.dashDistance * easedProgress)
                };

                // Check if we can move to target position (collision check)
                if (CanMoveTo(entityLoc, collider, targetPos, entityManager))
                {
                    transform.pos.x = targetPos.x;
                    transform.pos.y = targetPos.y;
                }
                else
                {
                    // Hit something - end dash early
                    playerComp.isDashing = false;
                    playerComp.dashTimer = 0.0f;
                    playerComp.dashCooldownTimer = playerComp.dashCooldown;
                    physicsComp.velocity = { 0.0f, 0.0f };
                }

                // Skip normal movement during dash
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

                // Zero out velocity during dash
                physicsComp.velocity = { 0.0f, 0.0f };

                // Skip normal movement this frame
                continue;
            }

            // ====== FORCE-BASED PHYSICS MOVEMENT ======

            // Clear forces from last frame
            physicsComp.ClearForces();

            // Apply movement force based on input (Force = direction * magnitude)
            Math::eVec2<float> input = GetMovementInput(inputManager);

            if (Math::eVec2SquaredLength(input) > 0.01f)
            {
                // Apply movement force in input direction
                // Force = movementForce * normalized_direction
                Math::eVec2<float> movementForceVec =
                {
                    input.x * playerComp.movementForce,
                    input.y * playerComp.movementForce
                };

                physicsComp.AddForce(movementForceVec);
            }

            // pply friction force (opposes velocity)
            // Friction force = -frictionCoeff * velocity
            // This creates smooth deceleration when no input is applied
            Math::eVec2<float> frictionForce;
            if (Math::eVec2SquaredLength(input) < 0.01f) {
                // Higher friction when no input
                frictionForce = {
                    -physicsComp.velocity.x * (playerComp.frictionCoefficient * 2.0f) * physicsComp.mass,
                    -physicsComp.velocity.y * (playerComp.frictionCoefficient * 2.0f) * physicsComp.mass
                };
            }
            else {
                // Normal friction during movement
                frictionForce = {
                    -physicsComp.velocity.x * playerComp.frictionCoefficient * physicsComp.mass,
                    -physicsComp.velocity.y * playerComp.frictionCoefficient * physicsComp.mass
                };
            }

            // If velocity is very low, just set it to zero
            if (Math::eVec2SquaredLength(physicsComp.velocity) < 0.1f) {
                physicsComp.velocity = { 0.0f, 0.0f };
            }
            physicsComp.AddForce(frictionForce);

            // Calculate acceleration from accumulated forces
            // Newton's Second Law: F = ma ? a = F/m
            physicsComp.acceleration.x = physicsComp.force.x / physicsComp.mass;
            physicsComp.acceleration.y = physicsComp.force.y / physicsComp.mass;

            // Integrate acceleration into velocity (Semi-implicit Euler)
            // v(t+dt) = v(t) + a * dt
            physicsComp.velocity.x += physicsComp.acceleration.x * deltaTime;
            physicsComp.velocity.y += physicsComp.acceleration.y * deltaTime;

            // Clamp velocity to max speed
            float speedSquared = Math::eVec2SquaredLength(physicsComp.velocity);
            float maxSpeedSquared = playerComp.maxSpeed * playerComp.maxSpeed;

            if (speedSquared > maxSpeedSquared)
            {
                float speed = sqrtf(speedSquared);
                physicsComp.velocity.x = (physicsComp.velocity.x / speed) * playerComp.maxSpeed;
                physicsComp.velocity.y = (physicsComp.velocity.y / speed) * playerComp.maxSpeed;
            }

            // Integrate velocity into position (with collision response)
            // p(t+dt) = p(t) + v * dt
            Math::eVec2<float> displacement =
            {
                physicsComp.velocity.x * deltaTime,
                physicsComp.velocity.y * deltaTime
            };

            // Axis-separated collision response with velocity modification
            // Try moving on both axes together first (diagonal movement)
            Math::eVec2<float> targetPosBoth =
            {
                transform.pos.x + displacement.x,
                transform.pos.y + displacement.y
            };

            // Check if the target position is valid (no collision with solid objects)
            if (CanMoveTo(entityLoc, collider, targetPosBoth, entityManager))
            {
                // Movement is valid - update transform position
                transform.pos.x = targetPosBoth.x;
                transform.pos.y = targetPosBoth.y;
            }
            else
            {
                // Diagonal movement blocked - try sliding along each axis independently

                // Try X-Axis movement only (horizontal sliding)
                Math::eVec2<float> targetPosX =
                {
                    transform.pos.x + displacement.x,
                    transform.pos.y  // Keep Y unchanged
                };

                if (CanMoveTo(entityLoc, collider, targetPosX, entityManager))
                {
                    transform.pos.x = targetPosX.x;
                }
                else
                {
                    // X-axis blocked - zero out X velocity (stop horizontal movement)
                    physicsComp.velocity.x = 0.0f;
                }

                // Try Y-Axis movement only (vertical sliding)
                Math::eVec2<float> targetPosY =
                {
                    transform.pos.x,  // Use current X (which may have just moved)
                    transform.pos.y + displacement.y
                };

                if (CanMoveTo(entityLoc, collider, targetPosY, entityManager))
                {
                    // Can move on Y-axis - update position
                    transform.pos.y = targetPosY.y;
                }
                else
                {
                    // Y-axis blocked - zero out Y velocity (stop vertical movement)
                    physicsComp.velocity.y = 0.0f;
                }

                // Player slides along the obstacle surface on whichever axis is free
                // If both axes blocked (stuck in corner), player stays in place
            }

            // STEP 8: Forces are automatically cleared at start of next frame
            // (see ClearForces() call at beginning of loop)
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
        // isKeyPressed() returns true while the key is held down
        if (inputManager.isKeyPressed(GLFW_KEY_W))
            input.y -= 1.0f;                       // Move up (negative Y in screen space)
        if (inputManager.isKeyPressed(GLFW_KEY_S))
            input.y += 1.0f;                       // Move down (positive Y in screen space)
        if (inputManager.isKeyPressed(GLFW_KEY_A))
            input.x -= 1.0f;                       // Move left (negative X)
        if (inputManager.isKeyPressed(GLFW_KEY_D))
            input.x += 1.0f;                       // Move right (positive X)

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

    /*============================================================================================
     * \brief       Check if player can move to target position without collision
     * \param[in]   ECS::EntityLoc const& playerEntity - Player entity location
     * \param[in]   Collision::CollisionComponent const& playerCollider - Player collision box
     * \param[in]   Math::eVec2<float> const& targetPos - Desired position to move to
     * \param[in]   ECS::EntityManager& entityManager - Entity manager for collision checks
     * \return      bool - True if movement is valid (no collision), false otherwise
     *===========================================================================================*/
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
        testTransform.pos.x = targetPos.x;
        testTransform.pos.y = targetPos.y;
        testTransform.rot = { 0.0f, 0.0f };             // Apply rotation
        testTransform.scale = { scaleSize, scaleSize }; // Apply scale

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
                else if (type == std::type_index(typeid(ECSComponents::CollisionComponent)))
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

            // Perform collision test at target position
            Collision::CollisionResult result = Collision::CollisionDetection::CheckCollision(
                playerCollider, testTransform,
                otherCollider, otherTransform
            );

            // If collision detected, can't move there
            if (result.hasCollision)
            {
                return false;
            }
        }

        // No collisions - movement is valid
        return true;
    }

    /*============================================================================================
     * \brief       Helper to retrieve Transform component from EntityLoc
     * \param[in]   ECS::EntityLoc const& entityLoc - Entity location in ECS
     * \return      ECSComponents::TransformComponent& - Reference to transform component
     *===========================================================================================*/
    ECSComponents::TransformComponent& PlayerSystem::GetTransform(const ECS::EntityLoc& entityLoc)
    {
        // Find the Transform component's index in this entity's archetype
        size_t index = entityLoc.archetype->findComponentIndex<ECSComponents::TransformComponent>();

        // Retrieve and return the component from the appropriate chunk
        return entityLoc.archetype->chunks_[entityLoc.chunkIndex]->retrieve<ECSComponents::TransformComponent>(
            index, entityLoc.entityIndexInChunk);
    }

    /*============================================================================================
     * \brief       Helper to retrieve Collision component from EntityLoc
     * \param[in]   ECS::EntityLoc const& entityLoc - Entity location in ECS
     * \return      Collision::CollisionComponent& - Reference to collision component
     *===========================================================================================*/
    Collision::CollisionComponent& PlayerSystem::GetCollision(const ECS::EntityLoc& entityLoc)
    {
        // Find the Collision component's index in this entity's archetype
        size_t index = entityLoc.archetype->findComponentIndex<ECSComponents::CollisionComponent>();

        // Retrieve and cast to Collision:: for compatibility with collision system
        // Both types have identical memory layout, so this cast is safe
        return reinterpret_cast<Collision::CollisionComponent&>(
            entityLoc.archetype->chunks_[entityLoc.chunkIndex]->retrieve<ECSComponents::CollisionComponent>(
                index, entityLoc.entityIndexInChunk)
            );
    }

} // namespace Player