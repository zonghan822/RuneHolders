#pragma once
#include <vector>
#include <string>
#include <algorithm>
#include <cstring>

#include "pathfinding/astar.hpp"
#include "state/state_machine.hpp"
#include "../event/event_bus.hpp"
#include "../ecs/entity.hpp"
#include "../component/components/ai_component.hpp"
#include "../component/components/physics_component.hpp"
#include "../component/components/transform.hpp"


namespace AI {

    class AIManager {
    public:
        AIManager(Event::EnoEventBus& bus);

        void update(ECS::EntityManager& entMng, float dt);

        void registerEntity(ECS::Entity& entity, ECS::EntityManager& entMng);

        void unregisterEntity(ECS::Entity& entity, ECS::EntityManager& entMng);

        AI::StateMachine& getStateMachine(ECS::Entity& entity);
        const AI::StateMachine& getStateMachine(ECS::Entity& entity) const;

        Path& getPath(ECS::Entity& entity);

        void removePath(ECS::Entity& entity) { paths_.erase(entity.id()); }


    private:
        Event::EnoEventBus& bus_;
        std::unordered_map<uint32_t, std::unique_ptr<AI::StateMachine>> stateMachines_;
        std::unordered_map<uint32_t, Path> paths_;
    };

}