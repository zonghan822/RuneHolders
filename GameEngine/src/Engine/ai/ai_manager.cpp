#include "ai_manager.hpp"

namespace AI {

    // -----------------------------
    // Constructor
    // -----------------------------
    AIManager::AIManager(Event::EnoEventBus& bus) : bus_{ bus } { /*empty by design*/ }


    // -----------------------------
    // Main Update Loop
    // -----------------------------
    void AIManager::update(ECS::EntityManager& entMng, float dt) {
        // Find all entities that have AIComponent
        std::vector<ECS::Entity> aiEntities = entMng.findEntity<ECSComponents::AIComponent>();

        for (ECS::Entity& entity : aiEntities) {
            auto& ai = entity.get<ECSComponents::AIComponent>(entMng);
            AI::StateMachine& sm = getStateMachine(entity);
            sm.update(entity, entMng, dt);

            // Handle transition requests
            if (std::strcmp(ai.nextState, ai.currentState) != 0){
                sm.changeState(ai.nextState, entity, entMng);
            }
        }
    }

    // ----------------------------------------
    // Register a new entity with AIComponent
    // ----------------------------------------
    void AIManager::registerEntity(ECS::Entity& entity, ECS::EntityManager& entMng) {
        uint32_t id = entity.id();
        if (stateMachines_.contains(id))
            return; // already registered

        auto sm = std::make_unique<StateMachine>();
        stateMachines_[id] = std::move(sm);
    }

    // -----------------------------
    // Unregister entity (cleanup)
    // -----------------------------
    void AIManager::unregisterEntity(ECS::Entity& entity, ECS::EntityManager& entMng) {
        stateMachines_.erase(entity.id());
    }

    // ------------------------------------------
    // Get StateMachine for a specific entity
    // ------------------------------------------
    AI::StateMachine& AIManager::getStateMachine(ECS::Entity& entity) {
        auto it = stateMachines_.find(entity.id());
        if (it == stateMachines_.end())
            throw std::runtime_error("Entity has no registered AI::StateMachine");
        return *it->second;
    }

    const AI::StateMachine& AIManager::getStateMachine(ECS::Entity& entity) const {
        auto it = stateMachines_.find(entity.id());
        if (it == stateMachines_.end())
            throw std::runtime_error("Entity has no registered AI::StateMachine");
        return *it->second;
    }

    AI::Path& AIManager::getPath(ECS::Entity& entity) {
        return paths_[entity.id()];
    }
}