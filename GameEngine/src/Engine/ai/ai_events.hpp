#pragma once
#include "../math/math.hpp"
#include <cstdint>

namespace AI {

    struct PathClearRequest {
        uint32_t entityID;
    };

    struct PathfindRequest {
        uint32_t entityID;
        Math::eVec2<float> start;
        Math::eVec2<float> goal;
        bool allowDiagonal;
        Math::eVec2<float>& next;
    };
}