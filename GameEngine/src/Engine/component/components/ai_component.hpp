#pragma once
#include <vector>
#include "../../math/math.hpp"

// Signals
#include "../../event/signals.hpp"

namespace ECSComponents
{
    struct AIComponent
    {
        // Current & Next State
        char currentState[32] = "Idle";
        char nextState[32] = "Idle";

        float awarenessRadius = 200.f;
        float chaseSpeed = 90.f;

        float attackRange = 40.f;
        float windupTime = 0.5f;
        float attackDuration = 0.3f;
        float hurtDuration = 0.4f;

        int health = 100;

        // Optional Script Callbacks
        bool useScriptCallbacks = false;

        char cbIdleEnter[64] = "";
        char cbIdleUpdate[64] = "";
        char cbIdleExit[64] = "";

        char cbChaseEnter[64] = "";
        char cbChaseUpdate[64] = "";
        char cbChaseExit[64] = "";

        char cbWindupEnter[64] = "";
        char cbWindupUpdate[64] = "";
        char cbWindupExit[64] = "";

        char cbAttackEnter[64] = "";
        char cbAttackUpdate[64] = "";
        char cbAttackExit[64] = "";

        char cbHurtEnter[64] = "";
        char cbHurtUpdate[64] = "";
        char cbHurtExit[64] = "";

        char cbDeathEnter[64] = "";
        char cbDeathUpdate[64] = "";
        char cbDeathExit[64] = "";

        // Signals
        std::unordered_map<std::string, Events::EnoSignal> signals_;


        template <typename V>
        void reflect(V&& v) const {
            v("currentState", currentState, "char32");
            v("nextState", nextState, "char32");

            v("awarenessRadius", awarenessRadius, "float");
            v("chaseSpeed", chaseSpeed, "float");

            v("attackRange", attackRange, "float");
            v("windupTime", windupTime, "float");
            v("attackDuration", attackDuration, "float");
            v("hurtDuration", hurtDuration, "float");

            v("health", health, "int");

            v("useScriptCallbacks", useScriptCallbacks, "bool");

            // Expose callbacks if desired
            v("cbIdleEnter", cbIdleEnter, "char64");
            v("cbIdleUpdate", cbIdleUpdate, "char64");
            v("cbIdleExit", cbIdleExit, "char64");

            v("cbChaseEnter", cbChaseEnter, "char64");
            v("cbChaseUpdate", cbChaseUpdate, "char64");
            v("cbChaseExit", cbChaseExit, "char64");

            v("cbWindupEnter", cbWindupEnter, "char64");
            v("cbWindupUpdate", cbWindupUpdate, "char64");
            v("cbWindupExit", cbWindupExit, "char64");

            v("cbAttackEnter", cbAttackEnter, "char64");
            v("cbAttackUpdate", cbAttackUpdate, "char64");
            v("cbAttackExit", cbAttackExit, "char64");

            v("cbHurtEnter", cbHurtEnter, "char64");
            v("cbHurtUpdate", cbHurtUpdate, "char64");
            v("cbHurtExit", cbHurtExit, "char64");

            v("cbDeathEnter", cbDeathEnter, "char64");
            v("cbDeathUpdate", cbDeathUpdate, "char64");
            v("cbDeathExit", cbDeathExit, "char64");
        }

        template <typename V>
        void reflect(V&& v) {
            v("currentState", currentState, "char32");
            v("nextState", nextState, "char32");

            v("awarenessRadius", awarenessRadius, "float");
            v("chaseSpeed", chaseSpeed, "float");

            v("attackRange", attackRange, "float");
            v("windupTime", windupTime, "float");
            v("attackDuration", attackDuration, "float");
            v("hurtDuration", hurtDuration, "float");

            v("health", health, "int");

            v("useScriptCallbacks", useScriptCallbacks, "bool");

            // Expose callbacks if desired
            v("cbIdleEnter", cbIdleEnter, "char64");
            v("cbIdleUpdate", cbIdleUpdate, "char64");
            v("cbIdleExit", cbIdleExit, "char64");

            v("cbChaseEnter", cbChaseEnter, "char64");
            v("cbChaseUpdate", cbChaseUpdate, "char64");
            v("cbChaseExit", cbChaseExit, "char64");

            v("cbWindupEnter", cbWindupEnter, "char64");
            v("cbWindupUpdate", cbWindupUpdate, "char64");
            v("cbWindupExit", cbWindupExit, "char64");

            v("cbAttackEnter", cbAttackEnter, "char64");
            v("cbAttackUpdate", cbAttackUpdate, "char64");
            v("cbAttackExit", cbAttackExit, "char64");

            v("cbHurtEnter", cbHurtEnter, "char64");
            v("cbHurtUpdate", cbHurtUpdate, "char64");
            v("cbHurtExit", cbHurtExit, "char64");

            v("cbDeathEnter", cbDeathEnter, "char64");
            v("cbDeathUpdate", cbDeathUpdate, "char64");
            v("cbDeathExit", cbDeathExit, "char64");
        }
    };
}