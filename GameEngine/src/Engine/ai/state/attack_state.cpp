#include "ai_state.hpp"

namespace AI {

    void AttackState::onEnter(ECS::Entity& e, ECS::EntityManager& m)
    {
        auto& ai = e.get<ECSComponents::AIComponent>(m);
        attackTimer_ = 0;

        if (ai.useScriptCallbacks)
            tryScript(e, m, ai.cbAttackEnter);

        // TODO: apply damage if player within range
    }

    void AttackState::onUpdate(ECS::Entity& e, ECS::EntityManager& m, float dt)
    {
        auto& ai = e.get<ECSComponents::AIComponent>(m);

        if (ai.useScriptCallbacks)
            tryScript(e, m, ai.cbAttackUpdate);

        attackTimer_ += dt;

        if (attackTimer_ >= ai.attackDuration)
            strcpy(ai.nextState, "Idle");
    }

    void AttackState::onExit(ECS::Entity& e, ECS::EntityManager& m)
    {
        auto& ai = e.get<ECSComponents::AIComponent>(m);

        if (ai.useScriptCallbacks)
            tryScript(e, m, ai.cbAttackExit);
    }

}