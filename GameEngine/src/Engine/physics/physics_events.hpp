/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         physics_events.hpp
* @author       Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   67%
* @co-author    ZongHan Law (zonghan.law@digipen.edu)   33%
* @brief        See file name.
*//*========================================================================================*/

#pragma once

namespace Physics {

    /** @brief Event fired by the physics system when two entities collide during integration. */
    struct CollideEvent {
        uint32_t entityA_ID;
        uint32_t entityB_ID;
    };

}