#include "ai_state.hpp"

namespace AI {

    void DeathState::onEnter(ECS::Entity& e, ECS::EntityManager& m)
    {
        auto& ai = e.get<ECSComponents::AIComponent>(m);

        if (ai.useScriptCallbacks)
            tryScript(e, m, ai.cbDeathEnter);

        // TODO: Disable physics, movement, etc. if needed

    }

    void DeathState::onUpdate(ECS::Entity& e, ECS::EntityManager& m, float dt)
    {
        (void)dt;
        auto& ai = e.get<ECSComponents::AIComponent>(m);

        if (ai.useScriptCallbacks)
            tryScript(e, m, ai.cbDeathUpdate);

        // Enemy is dead -> do nothing

    }

    void DeathState::onExit(ECS::Entity& e, ECS::EntityManager& m)
    {
        // Death usually never exits state
        auto& ai = e.get<ECSComponents::AIComponent>(m);
        if (ai.useScriptCallbacks)
            tryScript(e, m, ai.cbDeathExit);
    }

}