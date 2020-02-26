#pragma once

#include <algorithm>
#include <cmath>

namespace Trig {
    inline float sinFromCos(float cosTheta)
    {
        const float sin2Theta = 1.f - (cosTheta * cosTheta);
        return std::sqrt(std::max(0.f, sin2Theta));
    }

    inline float sinThetaFromCosTheta(float cosTheta)
    {
        const float result = std::sqrt(std::max(0.f, 1.f - cosTheta * cosTheta));
        return result;
    }

    inline float sin2FromCos(float cosTheta)
    {
        return std::max(0.f, 1.f - (cosTheta * cosTheta));
    }

    inline float cosFromSin2(float sin2Theta)
    {
        return std::sqrt(
            std::max(0.f, 1.f - sin2Theta)
        );
    }
};
