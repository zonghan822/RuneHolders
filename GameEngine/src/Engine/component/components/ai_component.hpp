/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         ai_component.hpp
* @author       ZongHan Law (zonghan.law@digipen.edu)   79%
* @co-author    Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   14%
* @brief        See file name.
*//*========================================================================================*/

#pragma once
#include <vector>
#include "../../math/math.hpp"

// Signals
#include "../../event/signals.hpp"

namespace ECSComponents
{
    struct AIComponent
    {
        float awarenessRadius = 200.f;
        float chaseSpeed = 90.f;
        bool  allowDiagonal  = true;
        int   pathClearance  = 1;   // wall-proximity cost radius for pathfinding


        // Signals
        std::unordered_map<std::string, Events::EnoSignal> signals_;


        template <typename V>
        void reflect(V&& v) const {
            v("awarenessRadius", awarenessRadius, "float");
            v("chaseSpeed", chaseSpeed, "float");
            v("allowDiagonal", allowDiagonal, "bool");
        }

        template <typename V>
        void reflect(V&& v) {
            v("awarenessRadius", awarenessRadius, "float");
            v("chaseSpeed", chaseSpeed, "float");
            v("allowDiagonal", allowDiagonal, "bool");
        }
    };
}