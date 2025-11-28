#pragma once
#include "../../math/math.hpp"
#include <cstdint>

namespace AI {

    struct PathClearRequest {
        uint32_t entityID;
    };

    struct PathfindRequest {
        uint32_t entityID;
        Math::eVec2<float> start;
        Math::eVec2<float> goal;
        Math::eVec2<float>& next;
    };

    struct ScriptEvent {
        uint32_t entityID;
        const char* methodName;
    };

    struct AttackEvent {
        uint32_t entityID;
    };
}