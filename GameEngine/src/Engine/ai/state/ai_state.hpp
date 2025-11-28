#pragma once
#include "../../ecs/entity.hpp"
#include "../../component/components/ai_component.hpp"
#include "../../component/components/physics_component.hpp"
#include "../../component/components/transform.hpp"
#include "ai_events.hpp"

namespace AI {

    class AIState {
    public:
        virtual ~AIState() = default;

        virtual void onEnter(ECS::Entity& entity, ECS::EntityManager& entMng) {}
        virtual void onUpdate(ECS::Entity& entity, ECS::EntityManager& entMng, float dt) = 0;
        virtual void onExit(ECS::Entity& entity, ECS::EntityManager& entMng) {}

    protected:
        void tryScript(ECS::Entity& e, ECS::EntityManager& m, const char* method) {
            if (!method || method[0] == 0) return;
            m.getEventBus().publish(ScriptEvent{ e.id(), method });
        }
    };

    class IdleState : public AIState {
    public:
        void onEnter(ECS::Entity& e, ECS::EntityManager& m) override;
        void onUpdate(ECS::Entity& e, ECS::EntityManager& m, float dt) override;
        void onExit(ECS::Entity& e, ECS::EntityManager& m) override;
    };

    class ChaseState : public AIState {
    public:
        void onEnter(ECS::Entity& e, ECS::EntityManager& m) override;
        void onUpdate(ECS::Entity& e, ECS::EntityManager& m, float dt) override;
        void onExit(ECS::Entity& e, ECS::EntityManager& m) override;
    };

    class WindupState : public AIState {
    public:
        void onEnter(ECS::Entity& e, ECS::EntityManager& m) override;
        void onUpdate(ECS::Entity& e, ECS::EntityManager& m, float dt) override;
        void onExit(ECS::Entity& e, ECS::EntityManager& m) override;
    private:
        float windupTimer_;
    };

    class AttackState : public AIState {
    public:
        void onEnter(ECS::Entity& e, ECS::EntityManager& m) override;
        void onUpdate(ECS::Entity& e, ECS::EntityManager& m, float dt) override;
        void onExit(ECS::Entity& e, ECS::EntityManager& m) override;
    private:
        float attackTimer_;
    };

    class HurtState : public AIState {
    public:
        void onEnter(ECS::Entity& e, ECS::EntityManager& m) override;
        void onUpdate(ECS::Entity& e, ECS::EntityManager& m, float dt) override;
        void onExit(ECS::Entity& e, ECS::EntityManager& m) override;
    private:
        float hurtTimer_;
    };

    class DeathState : public AIState {
    public:
        void onEnter(ECS::Entity& e, ECS::EntityManager& m) override;
        void onUpdate(ECS::Entity& e, ECS::EntityManager& m, float dt) override;
        void onExit(ECS::Entity& e, ECS::EntityManager& m) override;
    };
}