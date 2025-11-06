#pragma once
#include "../../ecs/entity.hpp"
#include "../../component/components/ai_component.hpp"
#include "../../component/components/physics_component.hpp"
#include "../../component/components/transform.hpp"

namespace AI {

    class AIAgent;

    class AIState {
    public:
        virtual ~AIState() = default;

        virtual void onEnter(ECS::Entity& entity, ECS::EntityManager& entMng) {}
        virtual void onUpdate(ECS::Entity& entity, ECS::EntityManager& entMng, float dt) = 0;
        virtual void onExit(ECS::Entity& entity, ECS::EntityManager& entMng) {}
    };

}