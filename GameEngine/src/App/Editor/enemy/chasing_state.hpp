#pragma once
#include <cstring>
#include "engine/eno_engine.hpp"

namespace AI {

    class ChaseState : public AIState
    {
    public:
        ChaseState(PathfindingSystem& nav, AIManager& aiMng)
            : navSystem(nav), aiManager(aiMng) {
        }

        void onEnter(ECS::Entity& entity, ECS::EntityManager& entMng) override
        {
            auto& ai = entity.get<ECSComponents::AIComponent>(entMng);
            std::strncpy(ai.currentState, "Chase", sizeof(ai.currentState));

            // locate target (player)
            ECS::Entity player = entMng.findEntity<ECSComponents::PlayerComponent>()[0];
            Math::eVec2<float> playerPos = player.get<ECSComponents::TransformComponent>(entMng).pos;
            ai.goal = playerPos;

            // build new path
            computePath(entity, entMng);
        }

        void onUpdate(ECS::Entity& entity, ECS::EntityManager& entMng, float dt) override
        {
            auto& ai = entity.get<ECSComponents::AIComponent>(entMng);
            auto& transform = entity.get<ECSComponents::TransformComponent>(entMng);
            auto& physics = entity.get<ECSComponents::PhysicsComponent>(entMng);

            auto& pathData = aiManager.getPath(entity);

            // recompute if empty
            if (pathData.nodes.empty())
                computePath(entity, entMng);

            followPath(entity, entMng, physics, dt);

            if (pathData.nodes.empty())
                std::strncpy(ai.nextState, "Idle", sizeof(ai.nextState));
        }

    private:
        PathfindingSystem& navSystem;
        AIManager& aiManager;

        void computePath(ECS::Entity& entity, ECS::EntityManager& entMng)
        {
            auto& ai = entity.get<ECSComponents::AIComponent>(entMng);
            auto& transform = entity.get<ECSComponents::TransformComponent>(entMng);

            Math::eVec2<int> start{ (int)transform.pos.x, (int)transform.pos.y };
            Math::eVec2<int> goal{ (int)ai.goal.x, (int)ai.goal.y };

            auto result = navSystem.findPath(start, goal);

            // store in path registry instead of component
            auto& pathData = aiManager.getPath(entity);
            pathData.nodes = result.nodes;
            pathData.completed = pathData.nodes.empty();
        }

        void followPath(ECS::Entity& entity,
            ECS::EntityManager& entMng,
            ECSComponents::PhysicsComponent& physics,
            float dt)
        {
            auto& pathData = aiManager.getPath(entity);
            auto& transform = entity.get<ECSComponents::TransformComponent>(entMng);
            auto& ai = entity.get<ECSComponents::AIComponent>(entMng);

            if (pathData.nodes.empty()) return;

            Math::eVec2<int> nextNode = pathData.nodes.front();
            Math::eVec2<float> target{ (float)nextNode.x, (float)nextNode.y };

            Math::eVec2<float> diff = target - transform.pos;
            float len = eVec2Length(diff);
            if (len < 0.001f) return;

            Math::eVec2<float> dir;
            eVec2Normalize(dir, diff);

            physics.velocity = dir * ai.speed;
            physics.ClampVelocity();

            if (len < 0.25f)
                pathData.nodes.erase(pathData.nodes.begin());
        }
    };

}