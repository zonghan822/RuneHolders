#include "state_machine.hpp"

namespace AI {

    void StateMachine::addState(const char* name, std::unique_ptr<AIState> state) {
        states_[name] = std::move(state);
    }

    void StateMachine::changeState(const char* name, ECS::Entity& entity, ECS::EntityManager& entMng) {
        if (current_)
            current_->onExit(entity, entMng);

        auto it = states_.find(name);
        if (it != states_.end())
        {
            current_ = it->second.get();
            current_->onEnter(entity, entMng);
            std::strncpy(currentName_, name, sizeof(currentName_));
        }
    }

    void StateMachine::update(ECS::Entity& entity, ECS::EntityManager& entMng, float dt) {
        if (current_)
            current_->onUpdate(entity, entMng, dt);
    }

}