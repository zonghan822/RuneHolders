#pragma once
#include <cstring>
#include "engine/eno_engine.hpp"

namespace AI {

    class IdleState : public AIState
    {
    public:

        void onEnter(ECS::Entity& entity, ECS::EntityManager& entMng) override
        {

        }

        void onUpdate(ECS::Entity& entity, ECS::EntityManager& entMng, float dt) override
        {
         
        }

    private:

    };

}