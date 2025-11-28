#include "ai_path_agent.hpp"
#include "../state/ai_events.hpp"

namespace AI {

    AIPathAgent::AIPathAgent(PathfindingSystem& pf)
        : pf_(pf)
    { }

    // Compute full path from start to goal
    void AIPathAgent::computePath(const Math::eVec2<int>& start, const Math::eVec2<int>& goal, bool allowDiagonal) {
        path_ = pf_.findPath(start, goal);
        pf_.setAllowDiagonal(allowDiagonal);
        currentIndex_ = 0;
    }

    // Remove saved path
    void AIPathAgent::clear() {
        path_.nodes.clear();
        currentIndex_ = 0;
    }

    bool AIPathAgent::hasPath() const {
        return !path_.nodes.empty() && currentIndex_ < path_.nodes.size();
    }

    // Step through nodes
    Math::eVec2<int> AIPathAgent::getCurrentTile() const {
        if (!hasPath()) return { -1, -1 };
        return path_.nodes[currentIndex_];
    }

    // Move to next tile in the path
    void AIPathAgent::advance() {
        if (hasPath()) currentIndex_++;
    }

    Math::eVec2<int> AIPathAgent::getGoal() const {
        if (!hasPath()) return Math::eVec2<int>{-1, -1};
        return path_.nodes.back();
    }

    // Full path access
    std::vector<Math::eVec2<int>>& AIPathAgent::nodes() { return path_.nodes; }
    const std::vector<Math::eVec2<int>>& AIPathAgent::nodes() const { return path_.nodes; }

}