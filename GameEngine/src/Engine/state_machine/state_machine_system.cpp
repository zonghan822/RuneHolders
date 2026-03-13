/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         state_machine_system.cpp
* @author       ZongHan Law (zonghan.law@digipen.edu)   67%
* @co-author    Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   17%
* @co-author    Koe Yun Ji (yunji.koe@digipen.edu)   17%
* @brief        See file name.
*//*========================================================================================*/

#include "state_machine_system.hpp"
#include "../editor/editor_events.hpp"

namespace FSM {

    constexpr uint32_t INVALID_ENTITY_ID = std::numeric_limits<uint32_t>::max();

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

        // Clear all state machines when the scene stops so stale entity IDs
        // don't persist across a scene reload
        bus_.subscribe<Editor::StopScene>(
            [this](const Editor::StopScene&) {
                this->clear();
            }
        );
    }

    size_t StateMachineSystem::findIndex(uint32_t entityID) const
    {
        for (size_t i = 0; i < smEntityIDs_.size(); ++i)
            if (smEntityIDs_[i] == entityID) return i;
        return SIZE_MAX;
    }

    void StateMachineSystem::update(Editor::EnoSceneTree& sceneTree, ECS::EntityManager& entityManager)
    {
        // Process pending state changes before the main loop
        for (auto& [entityID, newState] : pendingStateChanges_)
        {
            if (entityID == INVALID_ENTITY_ID) continue;
            Editor::Node* node = sceneTree.getNodeByEntityID(entityID);
            if (!node) continue;
            if (!node->entity_.has<ECSComponents::StateMachineComponent>(entityManager)) continue;

            size_t idx = findIndex(entityID);
            if (idx == SIZE_MAX) continue;

            std::wstring className;
            if (node->script_)
            {
                auto& origins = node->script_->origin();
                if (!origins.empty())
                    className = *origins.begin();
            }
            smInstances_[idx].changeState(newState.c_str(), node->entity_, entityManager, className, node->sceneObjectID_);
        }
        pendingStateChanges_.clear();

        // Update all active state machines.
        // Set updating_ = true so any removeStateMachine calls fired by scripts
        // are deferred to pendingRemovals_ instead of mutating the vectors in place.
        updating_ = true;
        for (size_t i = 0; i < smEntityIDs_.size(); ++i)
        {
            uint32_t entityID = smEntityIDs_[i];
            if (entityID == INVALID_ENTITY_ID) continue;

            Editor::Node* node = sceneTree.getNodeByEntityID(entityID);
            if (!node || !node->enabled_) continue;
            if (!node->entity_.has<ECSComponents::StateMachineComponent>(entityManager)) continue;

            auto& smComp = node->entity_.get<ECSComponents::StateMachineComponent>(entityManager);
            if (smComp.numStates == 0 || strcmp(smComp.currentState, "None") == 0) continue;

            std::wstring className;
            if (node->script_)
            {
                auto& origins = node->script_->origin();
                if (!origins.empty())
                    className = *origins.begin();
            }
            smInstances_[i].update(node->entity_, entityManager, className, node->sceneObjectID_);
        }
        updating_ = false;

        // Now safe to apply any removals that were deferred during the loop
        flushRemovals();
    }

    void StateMachineSystem::requestStateChange(uint32_t entityID, const std::string& newState)
    {
        if (entityID == INVALID_ENTITY_ID) return;
        pendingStateChanges_[entityID] = newState;
    }

    void StateMachineSystem::clear()
    {
        smEntityIDs_.clear();
        smInstances_.clear();
        pendingRemovals_.clear();
        pendingStateChanges_.clear();
        updating_ = false;
    }

    void StateMachineSystem::createStateMachine(uint32_t entityID)
    {
        if (entityID == INVALID_ENTITY_ID) return;
        if (findIndex(entityID) != SIZE_MAX) return; // already registered
        smEntityIDs_.push_back(entityID);
        smInstances_.emplace_back();
    }

    void StateMachineSystem::removeStateMachine(uint32_t entityID)
    {
        if (entityID == INVALID_ENTITY_ID) return;

        if (updating_)
        {
            // Defer: avoid mutating the vectors while the update loop is running
            for (uint32_t id : pendingRemovals_)
                if (id == entityID) return; // already queued
            pendingRemovals_.push_back(entityID);
            return;
        }

        size_t idx = findIndex(entityID);
        if (idx == SIZE_MAX) return; // not found, nothing to do

        // Swap-and-pop: O(1) removal without shifting elements
        size_t last = smEntityIDs_.size() - 1;
        if (idx != last)
        {
            smEntityIDs_[idx] = smEntityIDs_[last];
            smInstances_[idx] = std::move(smInstances_[last]);
        }
        smEntityIDs_.pop_back();
        smInstances_.pop_back();

        pendingStateChanges_.erase(entityID);
    }

    void StateMachineSystem::flushRemovals()
    {
        for (uint32_t id : pendingRemovals_)
            removeStateMachine(id); // updating_ is false here, so runs immediately
        pendingRemovals_.clear();
    }

}
