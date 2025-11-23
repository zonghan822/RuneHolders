#include "ai_manager.hpp"

namespace AI {

    AIManager::AIManager(Event::EnoEventBus& bus, PathfindingSystem& pathfinder)
        : bus_(bus), pathfinder_(pathfinder)
    { 
        // Auto-register when AIComponent added
        bus_.subscribe<ECS::ComponentAdded>(
            [this](const ECS::ComponentAdded& ev) {
                if (ev.type == std::type_index(typeid(ECSComponents::AIComponent)))
                    this->registerEntity(ev.entityID);
            }
        );

        // Auto-unregister when AIComponent removed
        bus_.subscribe<ECS::ComponentRemoved>(
            [this](const ECS::ComponentRemoved& ev) {
                if (ev.type == std::type_index(typeid(ECSComponents::AIComponent)))
                    this->unregisterEntity(ev.entityID);
            }
        );

        // Auto-clean when entity destroyed
        bus_.subscribe<ECS::EntityDestroyed>(
            [this](const ECS::EntityDestroyed& ev) {
                this->unregisterEntity(ev.entityID);
            }
        );
    }

    void AIManager::update(ECS::EntityManager& entMng, float dt) {
        // Iterate every entity with AIComponent
        auto entities = entMng.findEntity<ECSComponents::AIComponent>();

        for (auto& entity : entities) {
            auto& ai = entity.get<ECSComponents::AIComponent>(entMng);
            auto& machine = getStateMachine(entity);

            machine.update(entity, entMng, dt);

            if (std::strcmp(ai.nextState, ai.currentState) != 0) {
                machine.changeState(ai.nextState, entity, entMng);
            }
        }
    }

    void AIManager::registerEntity(uint32_t id) {

        // Create StateMachine if not exists
        if (!stateMachines_.contains(id))
            stateMachines_[id] = std::make_unique<StateMachine>();

        // Create PathAgent if not exists
        if (!pathAgents_.contains(id))
            pathAgents_[id] = std::make_unique<AIPathAgent>(pathfinder_);
    }

    void AIManager::unregisterEntity(uint32_t id) {
        stateMachines_.erase(id);
        pathAgents_.erase(id);
    }

    StateMachine& AIManager::getStateMachine(ECS::Entity& entity) {
        return *stateMachines_.at(entity.id());
    }

    AIPathAgent& AIManager::getPathAgent(ECS::Entity& entity) {
        return *pathAgents_.at(entity.id());
    }

    void AIManager::clearAll() {
        stateMachines_.clear();
        pathAgents_.clear();
    }
}