#pragma once
#include <unordered_map>
#include <cstring>
#include <string>
#include "../ecs/entity.hpp"

namespace FSM {

    class StateMachine {
    public:
        void changeState(const char* name, ECS::Entity& entity, ECS::EntityManager& entMng, std::wstring& className);

        void update(ECS::Entity& entity, ECS::EntityManager& entMng, std::wstring& className);
    };

}