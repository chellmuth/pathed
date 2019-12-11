#pragma once

#include <cmath>

namespace Trig {
    inline float sinFromCos(float cosTheta) {
        const float sin2Theta = 1.f - (cosTheta * cosTheta);
        return sqrtf(std::max(0.f, sin2Theta));
    }
};
