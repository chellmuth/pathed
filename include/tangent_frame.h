#pragma once

#include "util.h"
#include "vector.h"

#include <assert.h>
#include <cmath>

namespace TangentFrame {
    inline float cosTheta(const Vector3 &vector)
    {
        return vector.y();
    }

    inline float cos2Theta(const Vector3 &vector)
    {
        return vector.y() * vector.y();
    }

    inline float cos4Theta(const Vector3 &vector)
    {
        return vector.y() * vector.y() * vector.y() * vector.y();
    }

    inline float sinTheta(const Vector3 &vector)
    {
        return sqrtf(std::max(0.f, 1.f - cos2Theta(vector)));
    }

    inline float sin2Theta(const Vector3 &vector)
    {
        return 1.f - cos2Theta(vector);
    }

    inline float tanTheta(const Vector3 &vector)
    {
        return sinTheta(vector) / cosTheta(vector);
    }

    inline float tan2Theta(const Vector3 &vector)
    {
        return sin2Theta(vector) / cos2Theta(vector);
    }

    inline float cosPhi(const Vector3 &vector)
    {
        const float _sinTheta = sinTheta(vector);
        if (_sinTheta == 0.f) { return 1.f; }

        const float result = vector.x() / _sinTheta;
        return util::clamp(result, -1.f, 1.f);
    }

    inline float cos2Phi(const Vector3 &vector)
    {
        return cosPhi(vector) * cosPhi(vector);
    }

    inline float sinPhi(const Vector3 &vector)
    {
        const float _sinTheta = sinTheta(vector);
        if (_sinTheta == 0.f) { return 0.f; }

        const float result = vector.z() / _sinTheta;
        return util::clampClose(result, -1.f, 1.f);
    }

    inline float sin2Phi(const Vector3 &vector)
    {
        return sinPhi(vector) * sinPhi(vector);
    }
};
