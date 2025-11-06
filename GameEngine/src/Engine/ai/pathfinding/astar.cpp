#include "astar.hpp"

namespace AI {

    void PathfindingSystem::collectNeighbors(const Math::eVec2<int>& p, std::vector<Math::eVec2<int>>& out) const {
        out.clear();
        // 4-neighborhood
        static const int OFF4[4][2] = { {1,0},{-1,0},{0,1},{0,-1} };
        for (auto& d : OFF4) {
            int nx = p.x + d[0], ny = p.y + d[1];
            if (inBounds(nx, ny) && isWalkable(nx, ny)) out.push_back({ nx,ny });
        }
        if (allowDiagonal_) {
            // 4 diagonals (optional). If you want "no corner cutting", also require adjacent cardinals be walkable.
            static const int OFFD[4][2] = { {1,1},{1,-1},{-1,1},{-1,-1} };
            for (auto& d : OFFD) {
                int nx = p.x + d[0], ny = p.y + d[1];
                if (inBounds(nx, ny) && isWalkable(nx, ny)) out.push_back({ nx,ny });
            }
        }
    }

    Path PathfindingSystem::findPath(const Math::eVec2<int>& start, const Math::eVec2<int>& goal) {
        Path empty;
        if (!inBounds(start.x, start.y) || !inBounds(goal.x, goal.y)) return empty;
        if (!isWalkable(start.x, start.y) || !isWalkable(goal.x, goal.y)) return empty;

        const float INF = std::numeric_limits<float>::infinity();

        // g-score + closed + parent grids
        std::vector<std::vector<float>> g(h_, std::vector<float>(w_, INF));
        std::vector<std::vector<uint8_t>> closed(h_, std::vector<uint8_t>(w_, 0));
        std::vector<std::vector<Math::eVec2<int>>> parent(h_, std::vector<Math::eVec2<int>>(w_, { -1,-1 }));

        auto cost = [&](const Math::eVec2<int>& from, const Math::eVec2<int>& to) -> float {
            // 1.0 for cardinal, 1.4142 for diagonal (or 1 if you want uniform)
            int dx = std::abs(to.x - from.x);
            int dy = std::abs(to.y - from.y);
            return (dx + dy == 2) ? 1.41421356f : 1.0f;
            };

        std::priority_queue<Node, std::vector<Node>, NodeGreater> open;
        g[start.y][start.x] = 0.0f;
        float h0 = heuristic(start, goal);
        open.push({ start.x, start.y, h0, 0.0f });

        std::vector<Math::eVec2<int>> neigh;
        neigh.reserve(8);

        while (!open.empty()) {
            Node cur = open.top(); open.pop();
            if (closed[cur.y][cur.x]) continue;
            closed[cur.y][cur.x] = 1;

            if (cur.x == goal.x && cur.y == goal.y) {
                // Reconstruct
                Path path;
                Math::eVec2<int> p = goal;
                while (!(p.x == start.x && p.y == start.y)) {
                    path.nodes.emplace_back(p);
                    p = parent[p.y][p.x];
                    if (p.x == -1) { path.nodes.clear(); break; } // safety
                }
                if (!path.nodes.empty()) std::reverse(path.nodes.begin(), path.nodes.end());
                return path;
            }

            collectNeighbors({ cur.x,cur.y }, neigh);
            for (auto& nb : neigh) {
                if (closed[nb.y][nb.x]) continue;
                float tentative = g[cur.y][cur.x] + cost({ cur.x,cur.y }, nb);
                if (tentative < g[nb.y][nb.x]) {
                    g[nb.y][nb.x] = tentative;
                    parent[nb.y][nb.x] = { cur.x,cur.y };
                    float f = tentative + heuristic(nb, goal);
                    open.push({ nb.x, nb.y, f, tentative });
                }
            }
        }

        return empty; // no path
    }
}