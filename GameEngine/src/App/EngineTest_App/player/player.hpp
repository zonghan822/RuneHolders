/*************************************************
* @file player.hpp
* @brief Player component and system for character control
* @author Your Team
*
* This file contains:
*   - Player component (marks an entity as the player)
*   - Player system (handles movement with collision response)
*
* USAGE:
*   1. Create entity with Player, Transform, and Collision components
*   2. Create PlayerSystem and call Update() each frame
*   3. Player will move with WASD but respect collisions
**************************************************/

#pragma once

#include "../Engine/math/math.hpp"
#include "../Engine/component/components/transform.hpp"
#include "../Engine/collision/collision.hpp"
#include "../Engine/ecs/entity.hpp"
#include "../Engine/input/eno_input.hpp"
#include <GLFW/glfw3.h>

namespace Player
{
    //======================================================================================
    // PLAYER COMPONENT
    //======================================================================================

    /*
    * @brief Component that marks an entity as the player character
    *
    * Attach this to an entity to designate it as the player.
    * The entity should also have Transform and Collision components.
    */
    struct PlayerComponent
    {
        float moveSpeed;        // movement speed in pixels per second
        bool canMove;           // can the player move? (for stun effects, etc)

        /*
        * @brief Constructor with default values
        * @param speed - movement speed (default: 200.0f pixels/sec)
        */
        PlayerComponent(float speed = 200.0f)
            : moveSpeed(speed)
            , canMove(true)
        {
        }
    };

    //======================================================================================
    // PLAYER SYSTEM
    //======================================================================================

    /*
    * @brief System that handles player movement with collision response
    *
    * This system:
    *   - Reads WASD input for movement
    *   - Calculates desired position
    *   - Checks for collisions at new position
    *   - Only moves if no collision (collision response)
    *
    * Usage:
    *   PlayerSystem playerSystem;
    *   playerSystem.Update(entityManager, window, deltaTime);
    */
    class PlayerSystem
    {
    public:
        /*
        * @brief Constructor
        */
        PlayerSystem() = default;

        /*
        * @brief Main update function - call every frame
        *
        * Handles player input and movement with collision checking
        *
        * @param entityManager - reference to entity manager
        * @param inputManager - reference to input manager for reading controls
        * @param deltaTime - time since last frame (seconds)
        */
        void Update(ECS::EntityManager& entityManager, Input::EnoInputManager& inputManager, float deltaTime);
        /*
        * @brief Helper to get transform component from entity (made public for game_logic access)
        */
        ECSComponents::TransformComponent& GetTransform(const ECS::EntityLoc& entityLoc);

    private:
        /*
        * @brief Get movement input from WASD keys using EnoInputManager
        * @param inputManager - reference to the input manager
        * @return normalized movement vector
        */
        Math::eVec2<float> GetMovementInput(Input::EnoInputManager& inputManager);

        /*
        * @brief Check if player can move to a new position
        *
        * Tests if the player's collider would overlap with any
        * solid colliders at the target position
        *
        * @param playerEntity - the player entity location
        * @param playerCollider - player's collision component
        * @param targetPos - desired new position
        * @param entityManager - entity manager
        * @return true if position is valid (no collisions)
        */
        bool CanMoveTo(
            const ECS::EntityLoc& playerEntity,
            const Collision::CollisionComponent& playerCollider,
            const Math::eVec2<float>& targetPos,
            ECS::EntityManager& entityManager);

        /*
        * @brief Helper to get collision component from entity
        */
        Collision::CollisionComponent& GetCollision(const ECS::EntityLoc& entityLoc);
    };

} // namespace Player