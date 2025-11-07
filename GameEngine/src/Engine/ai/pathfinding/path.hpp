#pragma once

#include <vector>
#include "../../math/math.hpp"

namespace AI{

    struct Path {
        std::vector<Math::eVec2<int>> nodes;
        bool completed = false;
    };

}