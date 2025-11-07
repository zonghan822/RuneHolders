#pragma once
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
            ECS::Entity player = entMng.findEntity<ECSComponents::PlayerComponent>()[0];
            Math::eVec2<float> playerPos = player.get<ECSComponents::TransformComponent>(entMng).pos;
            ai.goal = playerPos;
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

            float len = eVec2Length(target - transform.pos);
            if (len < 0.001f) return;

            Math::eVec2<float> dir;
            eVec2Normalize(dir, target - transform.pos);

            physics.velocity = dir * ai.speed;
            physics.ClampVelocity();

            if (len < 0.2f)
                ai.path.erase(ai.path.begin());
        }
    };

}