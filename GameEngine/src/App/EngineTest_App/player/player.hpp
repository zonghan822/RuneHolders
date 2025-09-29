/*************************************************
* @file player.hpp
* @brief Player controller with collision resolution
* @author Kimberly Charlotte Madlangbayan Ambrosio (k.ambrosio)
*
* This file contains the player controller that handles:
*   - Player entity management
*   - WASD movement input
*   - Collision detection and resolution
*   - Player state management
*
* USAGE:
*   1. Create Player object with entity manager reference
*   2. Call Initialize() to create the player entity
*   3. Call Update(deltaTime) each frame
*   4. Call Render(graphics) to draw the player
**************************************************/

#pragma once

// egnine includes
#include "../Engine/ecs/entity.hpp"
#include "../Engine/component/components/transform.hpp"
#include "../Engine/collision/collision.hpp"
#include "../Engine/graphics/graphics.hpp"
#include "../Engine/math/math.hpp"

//GLFW for input
#include <cstdint>
#include <unordered_map>


