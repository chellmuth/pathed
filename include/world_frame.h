#pragma once

#include "vector.h"

#include <algorithm>

namespace WorldFrame {
    inline float cosTheta(Vector3 normal, Vector3 w)
    {
        return std::max(
            0.f,
            normal.dot(w)
        );
    }
};
