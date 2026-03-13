/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         ai_path_agent.hpp
* @author       ZongHan Law (zonghan.law@digipen.edu)   67%
* @co-author    Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   33%
* @brief        See file name.
*//*========================================================================================*/

#pragma once
#include "astar.hpp"
#include "../../math/math.hpp"

#include <vector>

namespace AI {

    class AIPathAgent {
    public:

        AIPathAgent(PathfindingSystem& pf);

        // Compute full path from start to goal
        void computePath(const Math::eVec2<int>& start, const Math::eVec2<int>& goal, bool allowDiagonal);

        // Remove saved path
        void clear();

        bool hasPath() const;

        // Step through nodes
        Math::eVec2<int> getCurrentTile() const;

        // Move to next tile in the path
        void advance();

        Math::eVec2<int> getGoal() const;

        // Full path access
        std::vector<Math::eVec2<int>>& nodes();
        const std::vector<Math::eVec2<int>>& nodes() const;

    private:
        PathfindingSystem& pf_;

        Path path_;
        size_t currentIndex_ = 0;

    };

} // namespace AI