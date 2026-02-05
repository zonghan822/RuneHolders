/*============================================================================================
* @file         ai_manager.hpp
* @author       Zong Han (zonghan.law@digipen.edu)
* @date         30-11-2025
* @brief        Sets up behavior for AI
*//*========================================================================================*/

#pragma once
#include <vector>
#include <string>
#include <algorithm>
#include <cstring>

#include "pathfinding/astar.hpp"
#include "pathfinding/ai_path_agent.hpp"

#include "../event/event_bus.hpp"
#include "../ecs/entity.hpp"
#include "../ecs/component_events.hpp"
#include "../component/components/ai_component.hpp"
#include "../component/components/drawInfo.hpp"
#include "../physics/physics_events.hpp"
#include "../math/math.hpp"


namespace AI {

    class AIManager {
    public:
        AIManager(Event::EnoEventBus& bus, PathfindingSystem& pathfinder, ECS::EntityManager& entMng);

        // Accessors
        AIPathAgent& getPathAgent(ECS::Entity& entity);
        PathfindingSystem& getPathfinder() { return pathfinder_; }

        // Cleanup on scene unload
        void clearAll();

    private:
        Event::EnoEventBus& bus_;
        PathfindingSystem& pathfinder_;
        ECS::EntityManager& entMng_;

        std::unordered_map<uint32_t, std::unique_ptr<AIPathAgent>> pathAgents_;

        // AI component lifecycle
        void registerEntity(uint32_t entityId);
        void unregisterEntity(uint32_t entityId);
    };

}