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
            AI::StateMachine* sm = getStateMachine(entity);
            if (!sm) continue;
            sm->update(entity, entMng, dt);

            // Handle transition requests
            if (std::strcmp(ai.nextState, ai.currentState) != 0){
                sm->changeState(ai.nextState, entity, entMng);
            }
        }
    }

    // ----------------------------------------
    // Register a new entity with AIComponent
    // ----------------------------------------
    void AIManager::registerEntity(ECS::Entity& entity, ECS::EntityManager& entMng) {
        auto id = entity.id(); // use entity ID as key
        if (stateMachines_.find(id) != stateMachines_.end())
            return; // already registered

        // Create and store a new state machine
        auto sm = std::make_unique<StateMachine>();
        auto* smPtr = sm.get();
        stateMachines_.emplace(id, std::move(sm));

        // Assign pointer to ECS component
        auto& ai = entity.get<ECSComponents::AIComponent>(entMng);
        ai.stateMachinePtr = smPtr;
    }

    // -----------------------------
    // Unregister entity (cleanup)
    // -----------------------------
    void AIManager::unregisterEntity(ECS::Entity& entity, ECS::EntityManager& entMng) {
        auto id = entity.id();
        auto it = stateMachines_.find(id);
        if (it != stateMachines_.end()) {
            // Automatically deleted because of std::unique_ptr
            stateMachines_.erase(it);
        }

        // Clear the pointer in the ECS component
        if (entity.has<ECSComponents::AIComponent>(entMng)) {
            auto& ai = entity.get<ECSComponents::AIComponent>(entMng);
            ai.stateMachinePtr = nullptr;
        }
    }

    // ------------------------------------------
    // Get StateMachine for a specific entity
    // ------------------------------------------
    AI::StateMachine* AIManager::getStateMachine(ECS::Entity& entity) {
        auto id = entity.id();
        auto it = stateMachines_.find(id);
        if (it != stateMachines_.end())
            return it->second.get();
        return nullptr;
    }
}