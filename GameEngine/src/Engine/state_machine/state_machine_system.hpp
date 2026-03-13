/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         state_machine_system.hpp
* @author       ZongHan Law (zonghan.law@digipen.edu)   60%
* @co-author    Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   40%
* @brief        See file name.
*//*========================================================================================*/

#pragma once
#include "state_machine.hpp"
#include "../editor/eno_scene_tree.hpp"
#include "../ecs/entity.hpp"
#include "../ecs/component_events.hpp"
#include "../event/event_bus.hpp"

#include <vector>
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

        // Parallel arrays: index i holds the same state machine entry.
        // Using vectors instead of unordered_map so removal during iteration
        // (via swap-and-pop) never invalidates the iterating index.
        std::vector<uint32_t>     smEntityIDs_;
        std::vector<StateMachine> smInstances_;

        // Removals requested while the update loop is active are deferred here
        // and flushed once the loop finishes, avoiding mid-iteration mutation.
        std::vector<uint32_t> pendingRemovals_;
        bool updating_ = false;

        // Pending state change requests from scripts (written outside the main loop)
        std::unordered_map<uint32_t, std::string> pendingStateChanges_;

        // Returns the index of entityID in smEntityIDs_, or SIZE_MAX if absent
        size_t findIndex(uint32_t entityID) const;

        void createStateMachine(uint32_t entityID);
        void removeStateMachine(uint32_t entityID);

        // Apply all deferred removals accumulated during the last update pass
        void flushRemovals();
    };

}