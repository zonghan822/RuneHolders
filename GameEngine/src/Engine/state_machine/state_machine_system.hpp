#pragma once
#include "state_machine.hpp"
#include "../editor/eno_scene_tree.hpp"
#include "../ecs/entity.hpp"
#include <unordered_map>

namespace FSM {

    // System to manage state machines for all entities
    class StateMachineSystem {
    public:
        StateMachineSystem() = default;

        // Update all entities with state machine components
        void update(Editor::EnoSceneTree& sceneTree, ECS::EntityManager& entityManager, float deltaTime);

        // Request a state change for an entity
        void requestStateChange(uint32_t nodeID, const std::string& newState);

        // Process pending state changes
        void processStateChanges(Editor::EnoSceneTree& sceneTree, ECS::EntityManager& entityManager);

        // Clean up state machine when entity is destroyed
        void removeStateMachine(uint32_t nodeID);

        // Clear all state machines (e.g., when loading new scene)
        void clear();

    private:
        // Map of node ID to state machine instance
        std::unordered_map<uint32_t, StateMachine> stateMachines_;

        // Pending state change requests from scripts
        std::unordered_map<uint32_t, std::string> pendingStateChanges_;

        // Get or create a state machine for an entity
        StateMachine& getOrCreateStateMachine(uint32_t nodeID);
    };

}