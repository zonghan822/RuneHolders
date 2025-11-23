#pragma once
#include <vector>
#include <queue>
#include <limits>
#include <cmath>
#include <cstdint>
#include <algorithm>

#include "../../math/math.hpp"
#include "../../component/components/transform.hpp"
#include "../../component/components/collision_component.hpp"
#include "../../ecs/entity.hpp"
#include "../../tilemap/tilemap.hpp"

#undef min
#undef max

namespace AI {

    struct Path {
        std::vector<Math::eVec2<int>> nodes;
        bool completed = false;
    };

    class PathfindingSystem {
    public:

        PathfindingSystem(Map::TileMapSystem& tilemap)
            : tilemap_(tilemap) {
        }

        void setAllowDiagonal(bool allow) { allowDiagonal_ = allow; }
        bool allowDiagonal() const { return allowDiagonal_; }

        Path findPath(const Math::eVec2<int>& start,
            const Math::eVec2<int>& goal);

    private:
        Map::TileMapSystem& tilemap_;
        bool allowDiagonal_ = false;

        struct Node {
            Math::eVec2<int> pos;
            int g, h;
            int f() const { return g + h; }
            bool operator>(const Node& rhs) const { return f() > rhs.f(); }
        };

        int heuristic(const Math::eVec2<int>& a, const Math::eVec2<int>& b) const;

        inline bool isWalkable(int x, int y) const {
            return tilemap_.isWalkable(x, y);
        }

        inline bool inBounds(int x, int y) const {
            return x >= 0 && y >= 0 &&
                x < tilemap_.width() &&
                y < tilemap_.height();
        }
    };

}
