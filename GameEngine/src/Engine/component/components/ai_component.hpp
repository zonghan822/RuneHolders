#pragma once
#include <vector>
#include "../../math/math.hpp"

namespace ECSComponents
{
    struct AIComponent
    {
        char currentState[32] = "Idle";
        char nextState[32] = "Idle";

        Math::eVec2<float> goal{};

        bool hasPath = false;
        float awarenessRadius = 200.f;
        bool chasingPlayer = false;
        float speed = 100.f;

        template <typename Visitor>
        void reflect(Visitor&& l)
        {
            l("currentState", currentState, "char32");
            l("nextState", nextState, "char32");
            l("goal", goal, "eVec2(float)");
            l("hasPath", hasPath, "bool");
            l("awarenessRadius", awarenessRadius, "float");
            l("chasingPlayer", chasingPlayer, "bool");
            l("speed", speed, "float");
        }

        template <typename Visitor>
        void reflect(Visitor&& l) const
        {
            l("currentState", currentState, "char32");
            l("nextState", nextState, "char32");
            l("goal", goal, "eVec2(float)");
            l("hasPath", hasPath, "bool");
            l("awarenessRadius", awarenessRadius, "float");
            l("chasingPlayer", chasingPlayer, "bool");
            l("speed", speed, "float");
        }
    };;
}