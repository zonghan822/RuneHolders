#include "ai_manager.hpp"
#include "ai_events.hpp"
#include "../component/components/state_machine_component.hpp"

namespace AI {

    AIManager::AIManager(Event::EnoEventBus& bus, PathfindingSystem& pathfinder, ECS::EntityManager& entMng)
        : bus_(bus), pathfinder_(pathfinder), entMng_{entMng}
    { 
        // Auto-register when AIComponent added
        bus_.subscribe<ECS::ComponentAdded>(
            [this](const ECS::ComponentAdded& ev) {
                if (ev.type == typeid(ECSComponents::AIComponent))
                    this->registerEntity(ev.entityID);
            }
        );

        // Auto-unregister when AIComponent removed
        bus_.subscribe<ECS::ComponentRemoved>(
            [this](const ECS::ComponentRemoved& ev) {
                if (ev.type == typeid(ECSComponents::AIComponent))
                    this->unregisterEntity(ev.entityID);
            }
        );

        // Auto-clean when entity destroyed
        bus_.subscribe<ECS::EntityDestroyed>(
            [this](const ECS::EntityDestroyed& ev) {
                this->unregisterEntity(ev.entityID);
            }
        );

        bus_.subscribe<PathfindRequest>(
            [&](const PathfindRequest& ev) {
                AIPathAgent& agent = *pathAgents_.at(ev.entityID);
                Math::eVec2<int> start = pathfinder_.worldToTile(ev.start);
                Math::eVec2<int> goal = pathfinder_.worldToTile(ev.goal);

                if (!agent.hasPath() || agent.getGoal() != goal)
                    agent.computePath(start, goal, ev.allowDiagonal);

                if (agent.hasPath()) {
                    ev.next = pathfinder_.tileToWorld(agent.getCurrentTile());

                    if (Math::eVec2Distance(ev.start, ev.next) < 2.f)
                        agent.advance();
                }
            }
        );
    }

    void AIManager::registerEntity(uint32_t id) {
        // Create PathAgent if not exists
        if (!pathAgents_.contains(id))
            pathAgents_[id] = std::make_unique<AIPathAgent>(pathfinder_);
    }

    void AIManager::unregisterEntity(uint32_t id) {
        pathAgents_.erase(id);
    }

    AIPathAgent& AIManager::getPathAgent(ECS::Entity& entity) {
        return *pathAgents_.at(entity.id());
    }

    void AIManager::clearAll() {
        pathAgents_.clear();
    }
}