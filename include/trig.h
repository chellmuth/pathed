#pragma once

#include <algorithm>
#include <cmath>

namespace Trig {
    inline float sinFromCos(float cosTheta)
    {
        const float sin2Theta = 1.f - (cosTheta * cosTheta);
        return sqrtf(std::max(0.f, sin2Theta));
    }

    inline float sinThetaFromCosTheta(float cosTheta)
    {
        const float result = sqrtf(std::max(0.f, 1.f - cosTheta * cosTheta));
        return result;
    }
};
