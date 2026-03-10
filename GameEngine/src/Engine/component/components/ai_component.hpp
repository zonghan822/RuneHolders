#pragma once
#include <vector>
#include "../../math/math.hpp"

// Signals
#include "../../event/signals.hpp"

namespace ECSComponents
{
    struct AIComponent
    {
        float awarenessRadius = 200.f;
        float chaseSpeed = 90.f;
        bool allowDiagonal = true;

        float attackRange = 40.f;
        float windupTime = 0.5f;
        float attackDuration = 0.3f;
        float hurtDuration = 0.4f;

        // Signals
        std::unordered_map<std::string, Events::EnoSignal> signals_;


        template <typename V>
        void reflect(V&& v) const {
            v("awarenessRadius", awarenessRadius, "float");
            v("chaseSpeed", chaseSpeed, "float");
            v("allowDiagonal", allowDiagonal, "bool");
            v("attackRange", attackRange, "float");
            v("windupTime", windupTime, "float");
            v("attackDuration", attackDuration, "float");
            v("hurtDuration", hurtDuration, "float");
        }

        template <typename V>
        void reflect(V&& v) {
            v("awarenessRadius", awarenessRadius, "float");
            v("chaseSpeed", chaseSpeed, "float");
            v("attackRange", attackRange, "float");
            v("windupTime", windupTime, "float");
            v("attackDuration", attackDuration, "float");
            v("hurtDuration", hurtDuration, "float");
        }
    };
}