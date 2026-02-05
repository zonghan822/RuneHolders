#pragma once
#include <unordered_map>
#include <cstring>
#include <string>
#include "../ecs/entity.hpp"

namespace FSM {

    class StateMachine {
    public:
        // scriptID is the sceneObjectID_ used to identify script instances
        void changeState(const char* name, ECS::Entity& entity, ECS::EntityManager& entMng,
                        std::wstring& className, uint32_t scriptID);

        void update(ECS::Entity& entity, ECS::EntityManager& entMng,
                   std::wstring& className, uint32_t scriptID);
    };

}