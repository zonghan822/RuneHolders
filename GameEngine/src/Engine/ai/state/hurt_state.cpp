#include "ai_state.hpp"

namespace AI {

    void HurtState::onEnter(ECS::Entity& e, ECS::EntityManager& m)
    {
        auto& ai = e.get<ECSComponents::AIComponent>(m);
        hurtTimer_ = 0.f;

        if (ai.useScriptCallbacks)
            tryScript(e, m, ai.cbHurtEnter);
    }

    void HurtState::onUpdate(ECS::Entity& e, ECS::EntityManager& m, float dt)
    {
        auto& ai = e.get<ECSComponents::AIComponent>(m);

        if (ai.useScriptCallbacks)
            tryScript(e, m, ai.cbHurtUpdate);

        hurtTimer_ += dt;

        if (ai.health <= 0) {
            strcpy(ai.nextState, "Death");
            return;
        }

        if (hurtTimer_ >= ai.hurtDuration)
            strcpy(ai.nextState, "Idle");
    }

    void HurtState::onExit(ECS::Entity& e, ECS::EntityManager& m)
    {
        auto& ai = e.get<ECSComponents::AIComponent>(m);

        if (ai.useScriptCallbacks)
            tryScript(e, m, ai.cbHurtExit);
    }

}