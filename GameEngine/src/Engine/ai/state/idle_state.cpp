#include "ai_state.hpp"

namespace AI {

    void IdleState::onEnter(ECS::Entity& e, ECS::EntityManager& m) {
        auto& ai = e.get<ECSComponents::AIComponent>(m);
        if (ai.useScriptCallbacks)
            tryScript(e, m, ai.cbIdleEnter);
    }

    void IdleState::onUpdate(ECS::Entity& e, ECS::EntityManager& m, float dt) {
        (void)dt;
        auto& ai = e.get<ECSComponents::AIComponent>(m);
        if (ai.useScriptCallbacks)
            tryScript(e, m, ai.cbIdleUpdate);

        auto& tf = e.get<ECSComponents::TransformComponent>(m);
        ECS::Entity player = m.findEntity<ECSComponents::PlayerComponent>()[0];
        auto& ptf = player.get<ECSComponents::TransformComponent>(m);

        float dist = Math::eVec2Length(ptf.pos - tf.pos);

        // Spot player -> Chase
        if (dist < ai.awarenessRadius) {
            strcpy(ai.nextState, "Chase");
            return;
        }
    }

    void IdleState::onExit(ECS::Entity& e, ECS::EntityManager& m) {
        auto& ai = e.get<ECSComponents::AIComponent>(m);
        if (ai.useScriptCallbacks)
            tryScript(e, m, ai.cbIdleExit);
    }
}