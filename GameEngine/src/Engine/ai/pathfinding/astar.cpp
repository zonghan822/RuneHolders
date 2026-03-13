/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         astar.cpp
* @author       ZongHan Law (zonghan.law@digipen.edu)   67%
* @co-author    Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   33%
* @brief        See file name.
*//*========================================================================================*/

#include "astar.hpp"

namespace AI {

    int PathfindingSystem::heuristic(const Math::eVec2<int>& a, const Math::eVec2<int>& b) const {
        int dx = std::abs(a.x - b.x);
        int dy = std::abs(a.y - b.y);
        if (allowDiagonal_) {
            // Octile heuristic
            return 10 * (dx + dy) + (14 - 20) * std::min(dx, dy);
        }
        else {
            // Manhattan heuristic
            return 10 * (dx + dy);
        }
    }

    Path PathfindingSystem::findPath(const Math::eVec2<int>& start,
        const Math::eVec2<int>& goal)
    {
        Path result;

        const int w = tilemap_.width();
        const int h = tilemap_.height();

        std::vector<std::vector<bool>> closed(h, std::vector<bool>(w, false));
        std::vector<std::vector<Math::eVec2<int>>> parent(
            h, std::vector<Math::eVec2<int>>(w, { -1, -1 })
        );

        std::priority_queue<Node, std::vector<Node>, std::greater<Node>> open;
        open.push({ start, 0, heuristic(start, goal) });

        // Cardinal directions
        static const Math::eVec2<int> cardinal[4] = {
            { 1,  0}, {-1,  0}, {0,  1}, {0, -1}
        };

        // Diagonal directions
        static const Math::eVec2<int> diagonal[4] = {
            { 1,  1}, { 1, -1}, {-1,  1}, {-1, -1}
        };

        while (!open.empty()) {
            Node cur = open.top();
            open.pop();

            if (closed[cur.pos.y][cur.pos.x])
                continue;
            closed[cur.pos.y][cur.pos.x] = true;

            if (cur.pos == goal) {
                Math::eVec2<int> p = goal;
                while (!(p == start)) {
                    result.nodes.push_back(p);
                    p = parent[p.y][p.x];
                }
                std::reverse(result.nodes.begin(), result.nodes.end());
                return result;
            }

            std::vector<Math::eVec2<int>> neighborList;

            // always push cardinal neighbors
            for (auto& d : cardinal) {
                neighborList.push_back(d);
            }

            // optionally push diagonal neighbors
            if (allowDiagonal_) {
                for (auto& d : diagonal) {
                    neighborList.push_back(d);
                }
            }

            for (auto& d : neighborList) {
                Math::eVec2<int> nxt = cur.pos + d;
                if (!inBounds(nxt.x, nxt.y)) continue;
                if (!isWalkable(nxt.x, nxt.y)) continue;
                if (closed[nxt.y][nxt.x]) continue;

                bool diagonalMove = (d.x != 0 && d.y != 0);
                if (diagonalMove) {
                    // Disallow diagonal if it cuts through a corner
                    if (!allowDiagonal_)
                        continue;

                    Math::eVec2<int> s1{ cur.pos.x + d.x, cur.pos.y };
                    Math::eVec2<int> s2{ cur.pos.x, cur.pos.y + d.y };

                    if (!isWalkable(s1.x, s1.y) || !isWalkable(s2.x, s2.y))
                        continue;
                }

                parent[nxt.y][nxt.x] = cur.pos;

                // Add more cost if move diagonally
                int moveCost = diagonalMove ? 14 : 10;

                open.push({
                    nxt,
                    cur.g + moveCost,
                    heuristic(nxt, goal)
                });
            }
        }

        return result;
    }

    Math::eVec2<int> PathfindingSystem::worldToTile(const Math::eVec2<float>& pos) const {
        return tilemap_.worldToTile(pos);
    }

    Math::eVec2<float> PathfindingSystem::tileToWorld(const Math::eVec2<int>& pos) const {
        return tilemap_.tileToWorld(pos.x, pos.y);
    }
}