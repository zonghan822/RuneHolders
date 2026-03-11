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


        // Signals
        std::unordered_map<std::string, Events::EnoSignal> signals_;


        template <typename V>
        void reflect(V&& v) const {
            v("awarenessRadius", awarenessRadius, "float");
            v("chaseSpeed", chaseSpeed, "float");
            v("allowDiagonal", allowDiagonal, "bool");
        }

        template <typename V>
        void reflect(V&& v) {
            v("awarenessRadius", awarenessRadius, "float");
            v("chaseSpeed", chaseSpeed, "float");
            v("allowDiagonal", allowDiagonal, "bool");
        }
    };
}