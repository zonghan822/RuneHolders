#pragma once
#include <cstring>
#include "engine/eno_engine.hpp"

namespace AI {

    class IdleState : public AIState {
    public:
        void onEnter(ECS::Entity& entity, ECS::EntityManager& entMng) override {
            auto& ai = entity.get<ECSComponents::AIComponent>(entMng);
            std::strncpy(ai.currentState, "Idle", sizeof(ai.currentState));
        }

        void onUpdate(ECS::Entity& entity, ECS::EntityManager& entMng, float dt) override {
            auto& ai = entity.get<ECSComponents::AIComponent>(entMng);
            auto& transform = entity.get<ECSComponents::TransformComponent>(entMng);

            // Placeholder: if we detect a target nearby, request transition
            if (detectPlayerNearby(transform, entMng)) {
                std::strncpy(ai.nextState, "Chase", sizeof(ai.nextState));
            }
        }

    private:
        bool detectPlayerNearby(const ECSComponents::TransformComponent& transform, ECS::EntityManager& entMng) {
            ECS::Entity player = entMng.findEntity<ECSComponents::PlayerComponent>()[0];
            Math::eVec2<float> playerPos = player.get<ECSComponents::TransformComponent>(entMng).pos;

            return true; // placeholder
        }
    };

}