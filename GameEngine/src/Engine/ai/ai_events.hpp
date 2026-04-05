/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         ai_events.hpp
* @author       Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   67%
* @co-author    ZongHan Law (zonghan.law@digipen.edu)   33%
* @brief        See file name.
*//*========================================================================================*/

#pragma once
#include "../math/math.hpp"
#include <cstdint>

namespace AI {

    /** @brief Event requesting that the stored path for an entity be cleared. */
    struct PathClearRequest {
        uint32_t entityID;
    };

    /** @brief Event requesting an A* pathfind from start to goal, returning the next waypoint. */
    struct PathfindRequest {
        uint32_t entityID;
        Math::eVec2<float> start;
        Math::eVec2<float> goal;
        bool allowDiagonal;
        Math::eVec2<float>& next;
    };
}