/*============================================================================================
* @file         player.cpp
* @author       Kimberly Charlotte Madlangbayan Ambrosio (k.ambrosio@digipen.edu)
* @date         03-11-2025
* @brief        Implementation of force-based player physics system. Uses proper Newtonian
*               physics: Force ? Acceleration ? Velocity ? Position. Works with both
*               PlayerComponent and PhysicsComponent for realistic movement.
*===========================================================================================*/

#include "player.hpp"
#include "input/eno_input_params.hpp"

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
     * \return      float - Eased value using formula: 1 - (1-x)�
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
    void PlayerSystem::Update(ECS::EntityManager& entityManager, Input::EnoInputManager& inputManager, float deltaTime, const Collision::CollisionSystem& collisionSystem)
    {
        const auto& spatialHash = collisionSystem.GetSpatialHash();
        auto playerEntities = entityManager.findEntity<
            ECSComponents::PlayerComponent,
            ECSComponents::PhysicsComponent,
            ECSComponents::TransformComponent,
            ECSComponents::CollisionComponent>();

        for (auto& entity : playerEntities)
        {
            auto& playerComp = entity.get<ECSComponents::PlayerComponent>(entityManager);
            auto& physicsComp = entity.get<ECSComponents::PhysicsComponent>(entityManager);

            if (!playerComp.canMove)
                continue;

            auto& transform = entity.get<ECSComponents::TransformComponent>(entityManager);
            auto& collider = reinterpret_cast<Collision::CollisionComponent&>(entity.get<ECSComponents::CollisionComponent>(entityManager));


            if (!playerComp.useScriptedMovement)
            {


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
                    if (CanMoveTo(entity.id(), collider, transform, targetPos, playerComp, entityManager, spatialHash))
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
                if (inputManager.isKeyPressed(Input::KeyCode::Space) && playerComp.dashCooldownTimer <= 0.0f && playerComp.canMove)
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
                            input = playerComp.defaultDashDir;  // Default dash down
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

                // Get movement input
                Math::eVec2<float> input = GetMovementInput(inputManager);

                if (Math::eVec2SquaredLength(input) > 0.01f)
                {
                    // Set velocity directly for instant, responsive movement (no acceleration)
                    physicsComp.velocity.x = input.x * playerComp.maxSpeed;
                    physicsComp.velocity.y = input.y * playerComp.maxSpeed;
                }
                else
                {
                    // No input - stop immediately
                    physicsComp.velocity = { 0.0f, 0.0f };
                }

                // Clear acceleration since we're setting velocity directly
                physicsComp.acceleration = { 0.0f, 0.0f };

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
            if (CanMoveTo(entity.id(), collider, transform, targetPosBoth, playerComp, entityManager, spatialHash))
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

                if (CanMoveTo(entity.id(), collider, transform, targetPosX, playerComp, entityManager, spatialHash))
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

                if (CanMoveTo(entity.id(), collider, transform, targetPosY, playerComp, entityManager, spatialHash))
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
        if (inputManager.isKeyPressed(Input::KeyCode::W))
            input.y -= 1.0f;                       // Move up (negative Y in screen space)
        if (inputManager.isKeyPressed(Input::KeyCode::S))
            input.y += 1.0f;                       // Move down (positive Y in screen space)
        if (inputManager.isKeyPressed(Input::KeyCode::A))
            input.x -= 1.0f;                       // Move left (negative X)
        if (inputManager.isKeyPressed(Input::KeyCode::D))
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
        uint32_t playerEntityID,
        const Collision::CollisionComponent& playerCollider,
        const ECSComponents::TransformComponent& playerTransform,
        const Math::eVec2<float>& targetPos,
        const ECSComponents::PlayerComponent& playerComp,
        ECS::EntityManager& entityManager,
        const Collision::SpatialHash& spatialHash)
    {
        // Create temporary transform at target position for collision testing
        ECSComponents::TransformComponent testTransform;
        testTransform.pos.x = targetPos.x;
        testTransform.pos.y = targetPos.y;
        testTransform.rot = playerTransform.rot;           // Apply rotation from player
        testTransform.scale = playerTransform.scale;       // Apply scale from player

        // Get player's half extents for the spatial hash quey
        float halfWidth = playerCollider.data.rectangle.width * 0.5f;
        float halfHeight = playerCollider.data.rectangle.height * 0.5f;

        // Query ONLY NEARBY entities instead of the entire world
        auto nearbyIDs = spatialHash.Query(targetPos, halfWidth, halfHeight);

        for (uint32_t otherID : nearbyIDs)
        {
            // Don't check collision with self
            if (otherID == playerEntityID)
                continue;

            ECS::Entity other{ otherID };

            // Make sure the entity has the components we need
            if (!other.has<ECSComponents::TransformComponent, ECSComponents::CollisionComponent>(entityManager)) continue;

            auto& otherTransform = other.get<ECSComponents::TransformComponent>(entityManager);
            auto& otherCollider = reinterpret_cast<Collision::CollisionComponent&>(other.get<ECSComponents::CollisionComponent>(entityManager));

            // Skip inactive colliders and triggers
            if (!otherCollider.isActive || otherCollider.isTrigger) continue;

            // Perform collision test at target position
            Collision::CollisionResult result = Collision::CollisionDetection::CheckCollision(
                playerCollider, testTransform,
                otherCollider, otherTransform);

            if (result.hasCollision)
            {
                // Check if it's a pushable box
                if (other.has<ECSComponents::PhysicsComponent, ECSComponents::BoxComponent>(entityManager))
                {
                    auto& otherPhysics = other.get<ECSComponents::PhysicsComponent>(entityManager);
                    auto& boxComp = other.get<ECSComponents::BoxComponent>(entityManager);

                    if (boxComp.isPushable && !otherPhysics.isKinematic)
                    {
                        // Calculate push direction (from player to box)
                        Math::eVec2<float> pushDir;
                        pushDir.x = otherTransform.pos.x - testTransform.pos.x;
                        pushDir.y = otherTransform.pos.y - testTransform.pos.y;

                        // Normalize direction
                        float length = sqrtf(pushDir.x * pushDir.x + pushDir.y * pushDir.y);
                        if (length > 0.0f)
                        {
                            pushDir.x /= length;
                            pushDir.y /= length;
                        }

                        // Apply push force (reduced by box resistance)
                        float pushMagnitude = playerComp.pushForce * (1.0f - boxComp.pushResistance);
                        otherPhysics.AddForce(Math::eVec2<float>{
                            pushDir.x* pushMagnitude,
                                pushDir.y* pushMagnitude
                        });
                    }
                }

                // Block movement
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