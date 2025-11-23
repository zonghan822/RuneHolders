#pragma once
#include "astar.hpp"
#include "../../math/math.hpp"

#include <vector>

namespace AI {

    class AIPathAgent {
    public:

        AIPathAgent(PathfindingSystem& pf);

        // Compute full path from start to goal
        void computePath(const Math::eVec2<int>& start, const Math::eVec2<int>& goal);

        // Remove saved path
        void clear();

        bool hasPath() const;

        // Step through nodes
        Math::eVec2<int> getCurrentTile() const;

        // Move to next tile in the path
        void advance();

        // Full path access
        std::vector<Math::eVec2<int>>& nodes() { return path_.nodes; }
        const std::vector<Math::eVec2<int>>& nodes() const { return path_.nodes; }

    private:
        PathfindingSystem& pf_;

        Path path_;
        size_t currentIndex_ = 0;
    };

} // namespace AI