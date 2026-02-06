#include "state_machine_system.hpp"
#include "../editor/editor_events.hpp"

namespace FSM {

    StateMachineSystem::StateMachineSystem(Event::EnoEventBus& bus) : bus_(bus)
    {
        // Auto-register when StateMachineComponent added
        bus_.subscribe<ECS::ComponentAdded>(
            [this](const ECS::ComponentAdded& ev) {
                if (ev.type == typeid(ECSComponents::StateMachineComponent))
                    this->createStateMachine(ev.entityID);
            }
        );

        // Auto-unregister when StateMachineComponent removed
        bus_.subscribe<ECS::ComponentRemoved>(
            [this](const ECS::ComponentRemoved& ev) {
                if (ev.type == typeid(ECSComponents::StateMachineComponent))
                    this->removeStateMachine(ev.entityID);
            }
        );

        // Auto-clean when entity destroyed
        bus_.subscribe<ECS::EntityDestroyed>(
            [this](const ECS::EntityDestroyed& ev) {
                this->removeStateMachine(ev.entityID);
            }
        );
    }

    // Update all entities with state machine components
    void StateMachineSystem::update(Editor::EnoSceneTree& sceneTree, ECS::EntityManager& entityManager) {
        // Process pending state changes first
        for (auto& [entityID, newState] : pendingStateChanges_) {
            Editor::Node* node = sceneTree.getNodeByEntityID(entityID);
            if (!node) continue;
            if (!node->script_) continue;
            if (!node->entity_.has<ECSComponents::StateMachineComponent>(entityManager)) continue;

            auto it = stateMachines_.find(entityID);
            if (it == stateMachines_.end()) continue;

            auto& origins = node->script_->origin();
            if (origins.empty()) continue;

            std::wstring className = *origins.begin();
            it->second.changeState(newState.c_str(), node->entity_, entityManager, className, node->sceneObjectID_);
        }
        pendingStateChanges_.clear();

        // Update all active state machines
        for (auto& [entityID, stateMachine] : stateMachines_) {
            Editor::Node* node = sceneTree.getNodeByEntityID(entityID);
            if (!node) continue;
            if (!node->enabled_) continue;
            if (!node->script_) continue;
            if (!node->entity_.has<ECSComponents::StateMachineComponent>(entityManager)) continue;

            auto& smComp = node->entity_.get<ECSComponents::StateMachineComponent>(entityManager);
            if (smComp.numStates == 0 || strcmp(smComp.currentState, "None") == 0) continue;

            auto& origins = node->script_->origin();
            if (origins.empty()) continue;

            std::wstring className = *origins.begin();
            stateMachine.update(node->entity_, entityManager, className, node->sceneObjectID_);
        }
    }

    // Request a state change for an entity
    void StateMachineSystem::requestStateChange(uint32_t entityID, const std::string& newState) {
        pendingStateChanges_[entityID] = newState;
    }

    // Clear all state machines (e.g., when loading new scene)
    void StateMachineSystem::clear() {
        stateMachines_.clear();
        pendingStateChanges_.clear();
    }

    // Create a state machine for an entity
    void StateMachineSystem::createStateMachine(uint32_t entityID) {
        if (!stateMachines_.contains(entityID)) {
            stateMachines_[entityID] = StateMachine();
        }
    }

    // Clean up state machine when entity is destroyed
    void StateMachineSystem::removeStateMachine(uint32_t entityID) {
        stateMachines_.erase(entityID);
        pendingStateChanges_.erase(entityID);
    }
}