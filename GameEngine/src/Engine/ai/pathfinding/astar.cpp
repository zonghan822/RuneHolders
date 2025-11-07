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

    void PathfindingSystem::update(ECS::EntityManager& entMng, float cellSize) {
        // Reset grid to walkable
        for (int y = 0; y < h_; ++y)
            for (int x = 0; x < w_; ++x)
                walkable_[y][x] = 1;

        // Find all entities with collision + transform
        std::vector<ECS::Entity> entities = entMng.findEntity<ECSComponents::CollisionComponent,
            ECSComponents::TransformComponent>();

        for (ECS::Entity& entity : entities)
        {
            auto& col = entity.get<ECSComponents::CollisionComponent>(entMng);
            auto& tr = entity.get<ECSComponents::TransformComponent>(entMng);

            // Skip triggers or inactive colliders
            if (!col.isActive || col.isTrigger)
                continue;

            // Convert world coordinates to grid indices
            auto markBlocked = [&](int gx, int gy)
                {
                    if (gx >= 0 && gy >= 0 && gx < w_ && gy < h_)
                        walkable_[gy][gx] = 0;
                };

            switch (col.type) {
            case ECSComponents::ColliderType::RECTANGLE:
            {
                float halfW = col.data.rectangle.width * 0.5f * tr.scale.x;
                float halfH = col.data.rectangle.height * 0.5f * tr.scale.y;
                float left = tr.pos.x - halfW + col.offset.x;
                float right = tr.pos.x + halfW + col.offset.x;
                float top = tr.pos.y - halfH + col.offset.y;
                float bottom = tr.pos.y + halfH + col.offset.y;

                int gx0 = static_cast<int>(left / cellSize);
                int gx1 = static_cast<int>(right / cellSize);
                int gy0 = static_cast<int>(top / cellSize);
                int gy1 = static_cast<int>(bottom / cellSize);

                for (int y = gy0; y <= gy1; ++y)
                    for (int x = gx0; x <= gx1; ++x)
                        markBlocked(x, y);
                break;
            }

            case ECSComponents::ColliderType::CIRCLE:
            {
                float r = col.data.circle.radius * tr.scale.x;
                Math::eVec2<float> center = tr.pos + col.offset;
                int gx0 = static_cast<int>((center.x - r) / cellSize);
                int gx1 = static_cast<int>((center.x + r) / cellSize);
                int gy0 = static_cast<int>((center.y - r) / cellSize);
                int gy1 = static_cast<int>((center.y + r) / cellSize);

                for (int y = gy0; y <= gy1; ++y)
                {
                    for (int x = gx0; x <= gx1; ++x)
                    {
                        float cx = (x + 0.5f) * cellSize;
                        float cy = (y + 0.5f) * cellSize;
                        float dx = cx - center.x;
                        float dy = cy - center.y;
                        if (dx * dx + dy * dy <= r * r)
                            markBlocked(x, y);
                    }
                }
                break;
            }

            case ECSComponents::ColliderType::LINE:
            {
                // Approximate: mark cells along the line segment as blocked
                Math::eVec2<float> p1 = col.data.line.p1;
                Math::eVec2<float> p2 = col.data.line.p2;
                int steps = static_cast<int>(std::ceil(Math::eVec2Length(p2 - p1) / cellSize));
                for (int i = 0; i <= steps; ++i)
                {
                    float t = i / static_cast<float>(steps);
                    Math::eVec2<float> p = p1 + (p2 - p1) * t;
                    markBlocked(static_cast<int>(p.x / cellSize),
                        static_cast<int>(p.y / cellSize));
                }
                break;
            }

            default:
                break;
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