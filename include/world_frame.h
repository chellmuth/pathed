#pragma once

#include "vector.h"

#include <algorithm>

namespace WorldFrame {
    inline float cosine(Vector3 vector1, Vector3 vector2)
    {
        return std::max(
            0.f,
            vector1.dot(vector2)
        );
    }
};
