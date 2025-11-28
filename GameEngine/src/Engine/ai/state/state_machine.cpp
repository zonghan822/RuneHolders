#include "state_machine.hpp"

namespace AI {

    void StateMachine::addState(const char* name, std::unique_ptr<AIState> state) {
        states_[name] = std::move(state);
    }

    void StateMachine::changeState(const char* name, ECS::Entity& entity, ECS::EntityManager& entMng) {
        if (strcmp(current_, "None") != 0)
            states_[current_]->onExit(entity, entMng);

        auto it = states_.find(name);
        if (it != states_.end())
        {
            std::strncpy(current_, name, sizeof(current_));
            states_[current_]->onEnter(entity, entMng);
        }
    }

    void StateMachine::update(ECS::Entity& entity, ECS::EntityManager& entMng, float dt) {
        if (strcmp(current_, "None") != 0) {
            std::cout << "State Machine updating: " << current_ << std::endl;
            states_[current_]->onUpdate(entity, entMng, dt);
        }
    }

}