#pragma once
#include <cmath>
#include <cstring>
#include "engine/eno_engine.hpp"

namespace AI {

    class ChaseState : public AIState
    {
    public:
        ChaseState(PathfindingSystem& nav) : navSystem(nav) {}

        void onEnter(ECS::Entity& entity, ECS::EntityManager& entMng) override
        {
            auto& ai = entity.get<ECSComponents::AIComponent>(entMng);
            std::strncpy(ai.currentState, "Chase", sizeof(ai.currentState));
            ai.hasPath = false;
        }

        void onUpdate(ECS::Entity& entity, ECS::EntityManager& entMng, float dt) override
        {
            auto& ai = entity.get<ECSComponents::AIComponent>(entMng);
            auto& transform = entity.get<ECSComponents::TransformComponent>(entMng);
            auto& physics = entity.get<ECSComponents::PhysicsComponent>(entMng);

            if (!ai.hasPath) {
                computePath(ai, transform);
            }

            followPath(ai, transform, physics, dt);

            if (ai.path.empty()) {
                std::strncpy(ai.nextState, "Idle", sizeof(ai.nextState));
            }
        }

    private:
        PathfindingSystem& navSystem;

        void computePath(ECSComponents::AIComponent& ai, const ECSComponents::TransformComponent& transform) {
            Math::eVec2<int> start = { (int)transform.pos.x, (int)transform.pos.y };
            Math::eVec2<int> goal = { (int)ai.goal.x, (int)ai.goal.y };
            auto path = navSystem.findPath(start, goal);
            ai.path = path.nodes;
            ai.hasPath = !ai.path.empty();
        }

        void followPath(ECSComponents::AIComponent& ai,
            ECSComponents::TransformComponent& transform,
            ECSComponents::PhysicsComponent& physics,
            float dt)
        {
            if (ai.path.empty()) return;

            Math::eVec2<int> next = ai.path.front();
            Math::eVec2<float> target{ (float)next.x, (float)next.y };

            Math::eVec2<float> dir{ target.x - transform.pos.x, target.y - transform.pos.y };
            float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
            if (len < 0.001f) return;

            dir.x /= len; dir.y /= len;

            physics.velocity.x = dir.x * 100.0f;
            physics.velocity.y = dir.y * 100.0f;
            physics.ClampVelocity();

            if (len < 0.2f)
                ai.path.erase(ai.path.begin());
        }
    };

}