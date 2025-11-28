#include "ai_state.hpp"

namespace AI {

    void ChaseState::onEnter(ECS::Entity& e, ECS::EntityManager& m) {
        auto& ai = e.get<ECSComponents::AIComponent>(m);
        if (ai.useScriptCallbacks) 
            tryScript(e, m, ai.cbChaseEnter);

        m.getEventBus().publish(PathClearRequest{ e.id() });
    }

    void ChaseState::onUpdate(ECS::Entity& e, ECS::EntityManager& m, float dt) {
        (void)dt;
        auto& ai = e.get<ECSComponents::AIComponent>(m);
        if (ai.useScriptCallbacks)
            tryScript(e, m, ai.cbChaseUpdate);

        auto& tf = e.get<ECSComponents::TransformComponent>(m);
        auto& physics = e.get<ECSComponents::PhysicsComponent>(m);

        ECS::Entity player = m.findEntity<ECSComponents::PlayerComponent>()[0];;
        auto& ptf = player.get<ECSComponents::TransformComponent>(m);

        float dist = Math::eVec2Length(ptf.pos - tf.pos);
        if (dist > ai.awarenessRadius * 2) {
            strcpy(ai.nextState, "Idle");
            return;
        }

        Math::eVec2<float> next{-1, -1};
        m.getEventBus().publish(PathfindRequest{
            e.id(),
            tf.pos,
            ptf.pos,
            next
        });

        Math::eVec2<float> dir;
        Math::eVec2Normalize(dir, next - tf.pos);
        physics.velocity = dir * ai.chaseSpeed;
        physics.ClampVelocity();


    }

    void ChaseState::onExit(ECS::Entity& e, ECS::EntityManager& m) {
        auto& ai = e.get<ECSComponents::AIComponent>(m);
        if (ai.useScriptCallbacks) 
            tryScript(e, m, ai.cbChaseExit);
    }

}