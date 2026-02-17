/*============================================================================================
* @file         player_config.hpp
* @author       Kimberly Charlotte Madlangbayan Ambrosio (k.ambrosio@digipen.edu)
* @date         03-11-2025
* @brief        Player configuration loader for force-based physics parameters. Reads player
*               stats from text files to allow easy tweaking without recompilation.
*===========================================================================================*/

#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include "../../../Engine/math/math.hpp"

namespace Player
{
    /*============================================================================================
     * \brief       Structure to hold all player configuration values loaded from file.
     *              Uses force-based physics parameters for proper Newtonian simulation.
     *===========================================================================================*/
    struct PlayerConfig
    {
        // Force-based movement parameters
        float movementForce;            // Force applied when moving (arbitrary units)
        float friction;
        float acceleration;
        float frictionCoefficient;      // Friction coefficient for drag (0-1)
        float maxSpeed;                 // Maximum movement speed (pixels/sec)
        float mass;                     // Player mass (affects acceleration)

        bool canMove;                   // Initial move state (usually true)

        // Sprite and collider dimensions
        float spriteWidth;              // Visual sprite width in pixels
        float spriteHeight;             // Visual sprite height in pixels
        float colliderWidth;            // Collision box width in pixels
        float colliderHeight;           // Collision box height in pixels

        // Dash properties
        float dashDuration;             // How long dash lasts in seconds
        float dashDistance;             // How far to dash in pixels
        float dashCooldown;             // Cooldown between dashes in seconds

        // Push parameters
        float pushForce;                // Base push force for pushing boxes
        float defaultDashDirX;          // Default dash direction X component
        float defaultDashDirY;          // Default dash direction Y component

        /*============================================================================================
         * \brief       Default constructor with fallback values if file loading fails
         *===========================================================================================*/
        PlayerConfig()
            : movementForce(800.0f)         // Force applied when moving
            , friction(0.0f)
            , acceleration(0.0f)
            , frictionCoefficient(0.8f)    // Friction for smooth deceleration
            , maxSpeed(200.0f)              // Max speed clamp
            , mass(1.0f)                    // Standard mass
            , canMove(true)
            , spriteWidth(48.0f)
            , spriteHeight(48.0f)
            , colliderWidth(48.0f)
            , colliderHeight(48.0f)
            , dashDuration(0.3f)
            , dashDistance(150.0f)
            , dashCooldown(0.5f)
            , pushForce(1500.0f)
            , defaultDashDirX(0.0f)
            , defaultDashDirY(1.0f)
        {
        }

        /*============================================================================================
         * \brief       Load configuration from text file
         * \param[in]   std::string const& filepath - Path to configuration file
         * \return      bool - True if loaded successfully, false if file not found/invalid
         *===========================================================================================*/
        bool LoadFromFile(const std::string& filepath)
        {
            std::ifstream file(filepath);
            if (!file.is_open())
            {
                return false;
            }

            std::string line;
            while (std::getline(file, line))
            {
                // Skip empty lines and comments (lines starting with #)
                if (line.empty() || line[0] == '#')
                    continue;

                // Parse key = value format
                std::istringstream iss(line);
                std::string key;
                char equals;
                float value;

                // Read: key = value
                if (iss >> key >> equals >> value)
                {
                    // Match key to corresponding member variable
                    if (key == "movementForce")
                        movementForce = value;
                    else if (key == "frictionCoefficient" || key == "friction")  // Support old name too
                        frictionCoefficient = value;
                    else if (key == "maxSpeed")
                        maxSpeed = value;
                    else if (key == "mass")
                        mass = value;
                    else if (key == "spriteWidth")
                        spriteWidth = value;
                    else if (key == "spriteHeight")
                        spriteHeight = value;
                    else if (key == "colliderWidth")
                        colliderWidth = value;
                    else if (key == "colliderHeight")
                        colliderHeight = value;
                    else if (key == "dashDuration")
                        dashDuration = value;
                    else if (key == "dashDistance")
                        dashDistance = value;
                    else if (key == "dashCooldown")
                        dashCooldown = value;
                    else if (key == "pushForce")
                        pushForce == value;
                    else if (key == "defaultDashDirX")
                        defaultDashDirX == value;
                    else if (key == "defaultDashDirY")
                        defaultDashDirY == value;
                }
                // Parse bool values (canMove = true/false)
                else
                {
                    std::istringstream iss2(line);
                    std::string boolValue;
                    if (iss2 >> key >> equals >> boolValue)
                    {
                        if (key == "canMove")
                        {
                            canMove = (boolValue == "true" || boolValue == "1");
                        }
                    }
                }
            }

            file.close();
            return true;
        }

        /*============================================================================================
         * \brief       Print current configuration values to console (for debugging)
         * \return      void
         *===========================================================================================*/
        void PrintConfig() const
        {
        }
    };

} // namespace Player