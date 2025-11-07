#pragma once
#include <unordered_map>
#include <memory>
#include <cstring>

#include "ai_state.hpp"

namespace AI {

    class StateMachine {
    public:
        void addState(const char* name, std::unique_ptr<AIState> state);
        void changeState(const char* name, ECS::Entity& entity, ECS::EntityManager& entMng);
        void update(ECS::Entity& entity, ECS::EntityManager& entMng, float dt);

    private:
        std::unordered_map<std::string, std::unique_ptr<AIState>> states_;
        char current_[32] = "None";
    };

}