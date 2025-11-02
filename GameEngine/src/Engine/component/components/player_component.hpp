/*============================================================================================
* @file         player_component.hpp
* @author       Kimberly Charlotte Madlangbayan Ambrosio (k.ambrosio)
* @date         02-11-2025
* @brief        Player component for ECS. Stores player-specific data including movement
*               physics, dash mechanics, and visual dimensions. This component marks an
*               entity as the player character.
*===========================================================================================*/

#pragma once

#include "../../math/math.hpp"

namespace ECSComponents
{
    /*============================================================================================
     * \brief       Component that marks an entity as the player character and stores all
     *              player-specific data including physics properties, dash mechanics, and
     *              sprite/collider dimensions
     *===========================================================================================*/
    struct PlayerComponent
    {
        // Physics properties (velocity-based movement)
        Math::eVec2<float> velocity;        // Current velocity in pixels per second
        float acceleration;                 // Acceleration when moving (pixels/sec^2)
        float friction;                     // Friction coefficient (0-1, higher = more friction)
        float maxSpeed;                     // Maximum movement speed (pixels/sec)

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
         * \param[in]   float speed - Movement speed (default: 200.0f pixels/sec)
         *===========================================================================================*/
        PlayerComponent(float speed = 200.0f)
            : velocity{ 0.0f, 0.0f }
            , acceleration(800.0f)          // Acceleration rate (pixels/sec^2)
            , friction(4.0f)                // Friction coefficient
            , maxSpeed(speed)               // Max speed
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