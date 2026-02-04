#pragma once
#include <unordered_map>
#include <cstring>
#include <string>
#include "../ecs/entity.hpp"

namespace FSM {

    class StateMachine {
    public:
        void changeState(const char* name, ECS::Entity& entity, ECS::EntityManager& entMng,
            const char* entityScriptPath);

        void update(ECS::Entity& entity, ECS::EntityManager& entMng,
            const char* entityScriptPath, float dt);
    };

}