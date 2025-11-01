/*============================================================================================
* @file         player.hpp
* @author       Kimberly Charlotte Madlangbayan Ambrosio (k.ambrosio@digipen.edu)
* @date         03-10-2025
* @brief        Player component and system for character control. Contains player component
*               that marks entities as the player and player system that handles movement
*               with collision response and dash mechanics.
*//*========================================================================================*/
#pragma once

#include "../Engine/math/math.hpp"
#include "../Engine/component/components/transform.hpp"
#include "../Engine/collision/collision.hpp"
#include "../Engine/ecs/entity.hpp"
#include "../Engine/input/eno_input.hpp"
#include "../Engine/engine/eno_engine.hpp"
#include "player_config.hpp"

#include <GLFW/glfw3.h>

namespace Player
{

    /*============================================================================================
    * \brief       Component that marks an entity as the player character. Attach this to an
    *              entity to designate it as the player. The entity should also have Transform
    *              and Collision components.
    *//*========================================================================================*/
    struct PlayerComponent
    {
        float moveSpeed;                    // Movement speed in pixels per second
        bool canMove;                       // Can the player move? (for stun effects, etc)

        // Sprite and collision dimensions (loaded from config instead of hardcoded constants)
        float spriteWidth;                  // Visual sprite width
        float spriteHeight;                 // Visual sprite height
        float colliderWidth;                // Collision box width
        float colliderHeight;               // Collision box height

        // Dashing properties
        bool isDashing;                     // Is the player currently dashing?
        float dashTimer;                    // Current time in the dash
        float dashDuration;                 // How long the dash lasts (e.g., 0.2 seconds)
        float dashDistance;                 // How far to dash (e.g, 150 pixels)
        float dashCooldown;                 // Cooldown time before next dash (e.g., 1.0 second)
        float dashCooldownTimer;            // Tracks cooldown
        Math::eVec2<float> dashDirection;   // Direction of the dash
        Math::eVec2<float> dashStartPos;    // Starting position of dash


        /*============================================================================================
        * \brief       Constructor with default values (FALLBACK if config not loaded)
        * \param[in]   float speed - Movement speed (default: 200.0f pixels/sec)
        *//*========================================================================================*/        
        PlayerComponent(float speed = 200.0f)
            : moveSpeed(speed)
            , canMove(true)
            , spriteWidth(48.0f)
            , spriteHeight(48.0f)
            , colliderWidth(48.0f)
            , colliderHeight(48.0f)
            , isDashing(false)
            , dashTimer(0.0f)
            , dashDuration(0.3f)            // Dash lasts 0.3 seconds
            , dashDistance(150.0f)          // Dash 150 pixels
            , dashCooldown(0.5f)            // 0.5 second cooldown
            , dashCooldownTimer(0.0f)
            , dashDirection{0.0f, 0.0f}
            , dashStartPos{0.0f, 0.0f}
        {
        }
        
        /*============================================================================================
        * \brief       Constructor from configuration file
        * \param[in]   PlayerConfig const& config - Configuration loaded from text file
        *//*========================================================================================*/
        PlayerComponent(const PlayerConfig& config)
            : moveSpeed(config.moveSpeed)
            , canMove(config.canMove)
            , spriteWidth(config.spriteWidth)
            , spriteHeight(config.spriteHeight)
            , colliderWidth(config.colliderWidth)
            , colliderHeight(config.colliderHeight)
            , isDashing(false)
            , dashTimer(0.0f)
            , dashDuration(config.dashDuration)
            , dashDistance(config.dashDistance)
            , dashCooldown(config.dashCooldown)
            , dashCooldownTimer(0.0f)
            , dashDirection{ 0.0f, 0.0f }
            , dashStartPos{ 0.0f, 0.0f }
        {
        }
    };

    /*============================================================================================
    * \brief       System that handles player movement with collision response. This system reads
    *              WASD input for movement, calculates desired position, checks for collisions at
    *              new position, and only moves if no collision (collision response). Also handles
    *              dash mechanics with spacebar input.
    *//*========================================================================================*/    
    class PlayerSystem
    {
    public:
        /*============================================================================================
        * \brief       Default constructor for PlayerSystem
        *//*========================================================================================*/        
        PlayerSystem() = default;

        /*============================================================================================
        * \brief       Main update function - call every frame. Handles player input and movement
        *              with collision checking.
        * \param[in]   ECS::EntityManager& entityManager - Reference to entity manager
        * \param[in]   Input::EnoInputManager& inputManager - Reference to input manager for reading controls
        * \param[in]   float deltaTime - Time since last frame in seconds
        * \return      void
        *//*========================================================================================*/        
        void Update(ECS::EntityManager& entityManager, Input::EnoInputManager& inputManager, float deltaTime);

        /*============================================================================================
        * \brief       Helper to get transform component from entity (made public for game_logic access)
        * \param[in]   ECS::EntityLoc const& entityLoc - Entity location information
        * \return      ECSComponents::TransformComponent& - Reference to entity's transform component
        *//*========================================================================================*/        
        ECSComponents::TransformComponent& GetTransform(const ECS::EntityLoc& entityLoc);

        /*============================================================================================
       * \brief       Helper to get collision component from entity location
       * \param[in]   ECS::EntityLoc const& entityLoc - Entity location information
       * \return      Collision::CollisionComponent& - Reference to entity's collision component
       *//*========================================================================================*/
        Collision::CollisionComponent& GetCollision(const ECS::EntityLoc& entityLoc);

    private:
        /*============================================================================================
        * \brief       Get movement input from WASD keys using EnoInputManager
        * \param[in]   Input::EnoInputManager& inputManager - Reference to the input manager
        * \return      Math::eVec2<float> - Normalized movement vector
        *//*========================================================================================*/
        Math::eVec2<float> GetMovementInput(Input::EnoInputManager& inputManager);

        /*============================================================================================
        * \brief       Check if player can move to a new position without colliding. Tests if the
        *              player's collider would overlap with any solid colliders at the target position.
        * \param[in]   ECS::EntityLoc const& playerEntity - The player entity location
        * \param[in]   Collision::CollisionComponent const& playerCollider - Player's collision component
        * \param[in]   Math::eVec2<float> const& targetPos - Desired new position
        * \param[in]   ECS::EntityManager& entityManager - Entity manager reference
        * \return      bool - True if position is valid (no collisions)
        *//*========================================================================================*/        
        bool CanMoveTo(
            const ECS::EntityLoc& playerEntity,
            const Collision::CollisionComponent& playerCollider,
            const Math::eVec2<float>& targetPos,
            ECS::EntityManager& entityManager);

       
    };

} // namespace Player