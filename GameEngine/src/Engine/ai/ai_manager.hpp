#pragma once
#include <vector>
#include <string>
#include <algorithm>
#include <cstring>

#include "state/ai_state.hpp"
#include "state/state_machine.hpp"

#include "pathfinding/astar.hpp"
#include "pathfinding/ai_path_agent.hpp"

#include "../event/event_bus.hpp"
#include "../ecs/entity.hpp"
#include "../ecs/component_events.hpp"
#include "../component/components/ai_component.hpp"
#include "../component/components/drawInfo.hpp"
#include "../physics/physics_events.hpp"
#include "../math/math.hpp"
#include "../graphics/animation/animation.hpp"


namespace AI {

    class AIManager {
    public:
        AIManager(Event::EnoEventBus& bus, PathfindingSystem& pathfinder, ECS::EntityManager& entMng, Animation::AnimationControllerRegistry& animRegistry);

        void update(float dt);

        // Accessors
        StateMachine& getStateMachine(ECS::Entity& entity);
        AIPathAgent& getPathAgent(ECS::Entity& entity);
        PathfindingSystem& getPathfinder() { return pathfinder_; }

        // Cleanup on scene unload
        void clearAll();

    private:
        Event::EnoEventBus& bus_;
        PathfindingSystem& pathfinder_;
        ECS::EntityManager& entMng_;
        Animation::AnimationControllerRegistry& animRegistry_;

        std::unordered_map<uint32_t, std::unique_ptr<StateMachine>> stateMachines_;
        std::unordered_map<uint32_t, std::unique_ptr<AIPathAgent>> pathAgents_;

        // AI component lifecycle
        void registerEntity(uint32_t entityId);
        void unregisterEntity(uint32_t entityId);

        bool handleCollisionForAI(uint32_t aiEntityID, uint32_t otherID);
    };

}