#include "state_machine_system.hpp"

namespace FSM {

    // Update all entities with state machine components
    void StateMachineSystem::update(Editor::EnoSceneTree& sceneTree, ECS::EntityManager& entityManager, float deltaTime) {
        // Get all nodes from scene tree
        auto nodes = sceneTree.getAllNodes();

        for (auto* node : nodes) {
            // Skip disabled nodes
            if (!node->enabled_) continue;

            // Skip nodes without scripts
            if (!node->script_) continue;

            // Skip entities without state machine component
            if (!node->entity_.has<ECSComponents::StateMachineComponent>(entityManager)) continue;

            // Get the state machine component
            auto& smComp = node->entity_.get<ECSComponents::StateMachineComponent>(entityManager);

            // Skip if no states defined or current state is "None"
            if (smComp.numStates == 0 || strcmp(smComp.currentState, "None") == 0) continue;

            // Get the state machine for this entity
            StateMachine& stateMachine = getOrCreateStateMachine(node->sceneObjectID_);

            // Update the state machine
            std::wstring& className = node->script_->name_;
            stateMachine.update(node->entity_, entityManager, className);
        }
    }

    // Request a state change for an entity
    void StateMachineSystem::requestStateChange(uint32_t nodeID, const std::string& newState) {
        pendingStateChanges_[nodeID] = newState;
    }

    // Process pending state changes
    void StateMachineSystem::processStateChanges(Editor::EnoSceneTree& sceneTree, ECS::EntityManager& entityManager) {
        for (auto& [nodeID, newState] : pendingStateChanges_) {
            // Find the node
            auto nodes = sceneTree.getAllNodes();
            Editor::Node* targetNode = nullptr;

            for (auto* node : nodes) {
                if (node->sceneObjectID_ == nodeID) {
                    targetNode = node;
                    break;
                }
            }

            if (!targetNode) continue;
            if (!targetNode->script_) continue;
            if (!targetNode->entity_.has<ECSComponents::StateMachineComponent>(entityManager)) continue;

            // Get state machine and change state
            StateMachine& stateMachine = getOrCreateStateMachine(nodeID);
            std::wstring& className = targetNode->script_->name_;

            stateMachine.changeState(newState.c_str(), targetNode->entity_,
                entityManager, className);
        }

        // Clear pending changes
        pendingStateChanges_.clear();
    }

    // Clean up state machine when entity is destroyed
    void StateMachineSystem::removeStateMachine(uint32_t nodeID) {
        stateMachines_.erase(nodeID);
        pendingStateChanges_.erase(nodeID);
    }

    // Clear all state machines (e.g., when loading new scene)
    void StateMachineSystem::clear() {
        stateMachines_.clear();
        pendingStateChanges_.clear();
    }


    // Get or create a state machine for an entity
    StateMachine& StateMachineSystem::getOrCreateStateMachine(uint32_t nodeID) {
        auto it = stateMachines_.find(nodeID);
        if (it == stateMachines_.end()) {
            stateMachines_[nodeID] = StateMachine();
        }
        return stateMachines_[nodeID];
    }

}