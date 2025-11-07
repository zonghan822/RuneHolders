#pragma once
#include <cstring>
#include "engine/eno_engine.hpp"

namespace AI {

    class IdleState : public AIState
    {
    public:
        IdleState(AIManager& aiMng, PathfindingSystem& nav)
            : aiManager(aiMng), navSystem(nav) {
        }

        void onEnter(ECS::Entity& entity, ECS::EntityManager& entMng) override
        {
            auto& ai = entity.get<ECSComponents::AIComponent>(entMng);
            std::strncpy(ai.currentState, "Idle", sizeof(ai.currentState));

            // Clear any leftover path
            aiManager.removePath(entity);
        }

        void onUpdate(ECS::Entity& entity, ECS::EntityManager& entMng, float dt) override
        {
            auto& ai = entity.get<ECSComponents::AIComponent>(entMng);
            auto& transform = entity.get<ECSComponents::TransformComponent>(entMng);

            // Detect player proximity (replace with your real detection logic)
            if (detectPlayerNearby(entity, entMng, ai.awarenessRadius))
            {
                ECS::Entity player = entMng.findEntity<ECSComponents::PlayerComponent>()[0];
                ai.goal = player.get<ECSComponents::TransformComponent>(entMng).pos;
                std::strncpy(ai.nextState, "Chase", sizeof(ai.nextState));
            }
        }

    private:
        AIManager& aiManager;
        PathfindingSystem& navSystem;

        bool detectPlayerNearby(ECS::Entity& entity, ECS::EntityManager& entMng, float radius)
        {
            auto& transform = entity.get<ECSComponents::TransformComponent>(entMng);
            auto players = entMng.findEntity<ECSComponents::PlayerComponent>();
            if (players.empty()) return false;

            auto& playerTr = players[0].get<ECSComponents::TransformComponent>(entMng);
            Math::eVec2<float> diff = playerTr.pos - transform.pos;
            return eVec2Length(diff) <= radius;
        }
    };

}