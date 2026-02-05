#include "state_machine.hpp"
#include "../event/event_bus.hpp"
#include "../eno_core_host/script_event.hpp"

namespace FSM {

    void StateMachine::changeState(const char* newState, ECS::Entity& entity,
        ECS::EntityManager& entMng, std::wstring& className, uint32_t scriptID) {

        // Get the component from the entity
        if (!entity.has<ECSComponents::StateMachineComponent>(entMng)) {
            return; // No state machine component
        }

        auto sm = entity.get<ECSComponents::StateMachineComponent>(entMng);

        if (strcmp(sm.currentState, newState) == 0) return;

        ECSComponents::StateInfo* next = sm.getState(newState);
        if (!next) return; // invalid state

        // Exit current state
        if (strcmp(sm.currentState, "None") != 0) {
            // Find current state info
            const ECSComponents::StateInfo* currentStateInfo = sm.getState(sm.currentState);

            if (currentStateInfo && currentStateInfo->onExitMethod[0] != '\0') {
                entMng.getEventBus().publish(Core::ScriptEvent{
                    scriptID,  // Use sceneObjectID_ for script instance lookup
                    className,
                    currentStateInfo->onExitMethod
                    });
            }
        }

        if (next) {
            // Update current state in component
            std::strncpy(sm.currentState, newState, sizeof(sm.currentState) - 1);
            sm.currentState[sizeof(sm.currentState) - 1] = '\0';

            // Call enter method
            if (next->onEnterMethod[0] != '\0') {
                entMng.getEventBus().publish(Core::ScriptEvent{
                    scriptID,  // Use sceneObjectID_ for script instance lookup
                    className,
                    next->onEnterMethod
                    });
            }
        }
    }

    void StateMachine::update(ECS::Entity& entity, ECS::EntityManager& entMng,
        std::wstring& className, uint32_t scriptID) {
        // Get the component from the entity
        if (!entity.has<ECSComponents::StateMachineComponent>(entMng)) {
            return; // No state machine component
        }

        auto& smComp = entity.get<ECSComponents::StateMachineComponent>(entMng);

        // Update current state
        if (strcmp(smComp.currentState, "None") != 0) {
            const ECSComponents::StateInfo* currentStateInfo = smComp.getState(smComp.currentState);

            if (currentStateInfo && currentStateInfo->onUpdateMethod[0] != '\0') {
                entMng.getEventBus().publish(Core::ScriptEvent{
                    scriptID,  // Use sceneObjectID_ for script instance lookup
                    className,
                    currentStateInfo->onUpdateMethod
                    });
            }
        }
    }
}