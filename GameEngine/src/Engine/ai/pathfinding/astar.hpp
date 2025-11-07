#pragma once
#include <vector>
#include <queue>
#include <limits>
#include <cmath>
#include <cstdint>
#include <algorithm>

#include "path.hpp"
#include "../../math/math.hpp"
#include "../../component/components/transform.hpp"
#include "../../component/components/collision_component.hpp"
#include "../../ecs/entity.hpp"

#undef min
#undef max

namespace AI {

    class PathfindingSystem {
    public:

        PathfindingSystem(int width, int height, bool allowDiagonal = false)
            : w_(width), h_(height), allowDiagonal_(allowDiagonal),
            walkable_(height, std::vector<uint8_t>(width, 1)) {
        }

        int  width()  const { return w_; }
        int  height() const { return h_; }

        void setWalkable(int x, int y, bool walkable) {
            if (inBounds(x, y)) walkable_[y][x] = walkable ? 1 : 0;
        }
        bool isWalkable(int x, int y) const {
            return inBounds(x, y) && walkable_[y][x] != 0;
        }

        void update(ECS::EntityManager& entMng, float cellSize);

        // Finds path from start to goal. Returns empty Path if none.
        Path findPath(const Math::eVec2<int>& start, const Math::eVec2<int>& goal);

    private:
        bool inBounds(int x, int y) const {
            return (x >= 0 && y >= 0 && x < w_ && y < h_);
        }

        float heuristic(const Math::eVec2<int>& a, const Math::eVec2<int>& b) const {
            // Manhattan if not diagonal; Chebyshev if diagonal enabled (works nicely on 8-neighborhood)
            if (!allowDiagonal_) return float(std::abs(a.x - b.x) + std::abs(a.y - b.y));
            return float(std::max(std::abs(a.x - b.x), std::abs(a.y - b.y)));
        }

        struct Node {
            int x, y;
            float f, g; // f = g + h
        };
        struct NodeGreater {
            bool operator()(const Node& a, const Node& b) const { return a.f > b.f; }
        };

        void collectNeighbors(const Math::eVec2<int>& p, std::vector<Math::eVec2<int>>& out) const;

        int w_, h_;
        bool allowDiagonal_;
        std::vector<std::vector<uint8_t>> walkable_; // 1 = walkable, 0 = blocked
    };

}
