/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         ai_manager.hpp
* @author       ZongHan Law (zonghan.law@digipen.edu)   67%
* @co-author    Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   17%
* @co-author    Koe Yun Ji (yunji.koe@digipen.edu)   17%
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

        /** @brief Retrieve the path agent associated with the given entity. */
        AIPathAgent& getPathAgent(ECS::Entity& entity);
        PathfindingSystem& getPathfinder() { return pathfinder_; }

        /** @brief Remove all registered path agents and reset AI state for scene unload. */
        void clearAll();

    private:
        Event::EnoEventBus& bus_;
        PathfindingSystem& pathfinder_;
        ECS::EntityManager& entMng_;

        std::unordered_map<uint32_t, std::unique_ptr<AIPathAgent>> pathAgents_;

        /** @brief Create a path agent for the entity and subscribe to its AI events. */
        void registerEntity(uint32_t entityId);
        /** @brief Destroy the path agent for the entity and clean up subscriptions. */
        void unregisterEntity(uint32_t entityId);
    };

}