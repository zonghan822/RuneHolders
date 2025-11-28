#include "ai_manager.hpp"

namespace AI {

    AIManager::AIManager(Event::EnoEventBus& bus, PathfindingSystem& pathfinder, ECS::EntityManager& entMng)
        : bus_(bus), pathfinder_(pathfinder), entMng_{entMng}
    { 
        // Auto-register when AIComponent added
        bus_.subscribe<ECS::ComponentAdded>(
            [this](const ECS::ComponentAdded& ev) {
                if (ev.type == typeid(ECSComponents::AIComponent))
                    this->registerEntity(ev.entityID);
            }
        );

        // Auto-unregister when AIComponent removed
        bus_.subscribe<ECS::ComponentRemoved>(
            [this](const ECS::ComponentRemoved& ev) {
                if (ev.type == typeid(ECSComponents::AIComponent))
                    this->unregisterEntity(ev.entityID);
            }
        );

        // Auto-clean when entity destroyed
        bus_.subscribe<ECS::EntityDestroyed>(
            [this](const ECS::EntityDestroyed& ev) {
                this->unregisterEntity(ev.entityID);
            }
        );

        bus_.subscribe<PathfindRequest>(
            [&](const PathfindRequest& ev) {
                AIPathAgent& agent = *pathAgents_.at(ev.entityID);
                Math::eVec2<int> start = pathfinder_.worldToTile(ev.start);
                Math::eVec2<int> goal = pathfinder_.worldToTile(ev.goal);

                if (!agent.hasPath() || agent.getGoal() != goal)
                    agent.computePath(start, goal, ev.allowDiagonal);

                if (agent.hasPath()) {
                    ev.next = pathfinder_.tileToWorld(agent.getCurrentTile());

                    if (Math::eVec2Distance(ev.start, ev.next) < 2.f)
                        agent.advance();
                }
            }
        );

        bus_.subscribe<Collision::CollisionEvent>(
            [&](const Collision::CollisionEvent& ev) {
                if (!handleCollisionForAI(ev.entityA_ID, ev.entityB_ID))
                    handleCollisionForAI(ev.entityB_ID, ev.entityA_ID);
            }
        );
    }

    void AIManager::update(float dt) {
        // Iterate every entity with AIComponent
        auto entities = entMng_.findEntity<ECSComponents::AIComponent>();

        for (auto& entity : entities) {
            auto& ai = entity.get<ECSComponents::AIComponent>(entMng_);
            auto& machine = getStateMachine(entity);

            if (std::strcmp(ai.nextState, ai.currentState) != 0) {
                machine.changeState(ai.nextState, entity, entMng_);
            }
            machine.update(entity, entMng_, dt);
        }
    }

    void AIManager::registerEntity(uint32_t id) {

        // Create StateMachine if not exists
        if (!stateMachines_.contains(id)) {
            auto sm = std::make_unique<StateMachine>();

            sm->addState("Idle", std::make_unique<IdleState>());
            sm->addState("Chase", std::make_unique<ChaseState>());
            sm->addState("Windup", std::make_unique<WindupState>());
            sm->addState("Attack", std::make_unique<AttackState>());
            sm->addState("Hurt", std::make_unique<HurtState>());
            sm->addState("Death", std::make_unique<DeathState>());

            stateMachines_[id] = std::move(sm);

            ECS::Entity aiEntity(id);
            auto& ai = aiEntity.get<ECSComponents::AIComponent>(entMng_);
            strcpy(ai.nextState, "Idle");
        }
        
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

    bool AIManager::handleCollisionForAI(uint32_t aiEntityID, uint32_t otherID) {
        // Must have AIComponent
        ECS::Entity aiEntity(aiEntityID);
        if (!aiEntity.has<ECSComponents::AIComponent>(entMng_))
            return false;

        // Check if the other entity is the player
        ECS::Entity other(otherID);
        if (!other.has<ECSComponents::PlayerComponent>(entMng_))
            return false;

        // Get AI component
        auto& ai = aiEntity.get<ECSComponents::AIComponent>(entMng_);

        // Ignore if already winding or attacking
        if (strcmp(ai.currentState, "Windup") == 0 ||
            strcmp(ai.currentState, "Attack") == 0)
        {
            return true;
        }

        // Don't trigger attack if AI is dead
        if (strcmp(ai.currentState, "Death") == 0)
            return true;

        // Switch to Windup
        strcpy(ai.nextState, "Windup");

        return true;
    }
}