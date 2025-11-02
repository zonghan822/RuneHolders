/*============================================================================================
* @file         player_component.hpp
* @author       Kimberly Charlotte Madlangbayan Ambrosio (k.ambrosio@digipen.edu)
* @date         03-11-2025
* @brief        Player component for ECS. Stores player-specific force-based movement
*               parameters, dash mechanics, and visual dimensions. Works with PhysicsComponent
*               for proper Newtonian physics (Force ? Acceleration ? Velocity ? Position).
*===========================================================================================*/

#pragma once

#include "../../../Engine/math/math.hpp"

namespace ECSComponents
{
    /*============================================================================================
     * \brief       Component that marks an entity as the player character. Stores player-specific
     *              force-based movement parameters and dash mechanics. Velocity/acceleration/force
     *              are stored in PhysicsComponent for proper physics simulation.
     *===========================================================================================*/
    struct PlayerComponent
    {
        // Force-based movement parameters (works with PhysicsComponent)
        // NOTE: Actual velocity, acceleration, and forces live in PhysicsComponent
        // These are player-specific tuning parameters
        Math::eVec2<float> velocity;
        float movementForce;                // Force magnitude applied when moving (N or arbitrary units)
        float acceleration;                 // Acceleration rate (pixels/sec^2)
        float friction;                     // Friction coefficient for drag and deceleration
        float mass;                         // Player mass (affects acceleration calculation)
        float maxSpeed;                     // Maximum movement speed clamp (pixels/sec)
        float frictionCoefficient;          // Friction coefficient for drag (0-1, higher = more friction)

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
         * \brief       Default constructor with fallback values
         * \param[in]   float speed - Max movement speed (default: 200.0f pixels/sec)
         *===========================================================================================*/
        PlayerComponent(float speed = 200.0f)
            : velocity{ 0.0f, 0.0f }
            , movementForce(800.0f)         // Force applied when moving
            , acceleration(800.0f)          // Acceleration rate
            , friction(0.15f)               // Friction coefficient
            , mass(1.0f)                    // Player mass
            , maxSpeed(speed)               // Max speed clamp
            , frictionCoefficient(0.15f)    // Friction for smooth deceleration
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
            , dashDirection{ 0.0f, 0.0f }
            , dashStartPos{ 0.0f, 0.0f }
        {
        }
    };

} // namespace ECSComponents