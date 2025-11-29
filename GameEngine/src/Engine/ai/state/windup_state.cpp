#include "ai_state.hpp"

namespace AI {

    void WindupState::onEnter(ECS::Entity& e, ECS::EntityManager& m)
    {
        auto& ai = e.get<ECSComponents::AIComponent>(m);
        windupTimer_ = 0;

        if (ai.useScriptCallbacks)
            tryScript(e, m, ai.cbWindupEnter);
    }

    void WindupState::onUpdate(ECS::Entity& e, ECS::EntityManager& m, float dt)
    {
        auto& ai = e.get<ECSComponents::AIComponent>(m);

        if (ai.useScriptCallbacks)
            tryScript(e, m, ai.cbWindupUpdate);

        windupTimer_ += dt;

        if (windupTimer_ >= ai.windupTime)
            strcpy(ai.nextState, "attack");
    }
            //strcpy(ai.nextState, "Attack");

    void WindupState::onExit(ECS::Entity& e, ECS::EntityManager& m)
    {
        auto& ai = e.get<ECSComponents::AIComponent>(m);

        if (ai.useScriptCallbacks)
            tryScript(e, m, ai.cbWindupExit);
    }

}