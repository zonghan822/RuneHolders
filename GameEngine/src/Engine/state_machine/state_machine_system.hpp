#pragma once
#include "state_machine.hpp"
#include "../editor/eno_scene_tree.hpp"
#include "../ecs/entity.hpp"
#include "../ecs/component_events.hpp"
#include "../event/event_bus.hpp"

#include <unordered_map>

namespace FSM {

    // System to manage state machines for all entities
    class StateMachineSystem {
    public:
        StateMachineSystem(Event::EnoEventBus& bus);

        // Update all entities with state machine components
        void update(Editor::EnoSceneTree& sceneTree, ECS::EntityManager& entityManager);

        // Request a state change for an entity
        void requestStateChange(uint32_t entityID, const std::string& newState);

        // Clear all state machines (e.g., when loading new scene)
        void clear();

    private:
        Event::EnoEventBus& bus_;

        // Map of entity ID to state machine instance
        std::unordered_map<uint32_t, StateMachine> stateMachines_;

        // Pending state change requests from scripts
        std::unordered_map<uint32_t, std::string> pendingStateChanges_;

        // Create a state machine for an entity
        void createStateMachine(uint32_t entityID);

        // Clean up state machine when entity is destroyed
        void removeStateMachine(uint32_t entityID);
    };

}