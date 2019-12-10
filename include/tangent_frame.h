#pragma once

#include "vector.h"

#include <assert.h>
#include <cmath>

namespace TangentFrame {
    float cosTheta(const Vector3 &vector)
    {
        return vector.y();
    }

    float cos2Theta(const Vector3 &vector)
    {
        return vector.y() * vector.y();
    }

    float cos4Theta(const Vector3 &vector)
    {
        return vector.y() * vector.y() * vector.y() * vector.y();
    }

    float sinTheta(const Vector3 &vector)
    {
        return sqrtf(1.f - cos2Theta(vector));
    }

    float sin2Theta(const Vector3 &vector)
    {
        return 1.f - cos2Theta(vector);
    }

    float tanTheta(const Vector3 &vector)
    {
        return sinTheta(vector) / cosTheta(vector);
    }

    float tan2Theta(const Vector3 &vector)
    {
        return sin2Theta(vector) / cos2Theta(vector);
    }

    float cosPhi(const Vector3 &vector)
    {
        const float _sinTheta = sinTheta(vector);
        if (_sinTheta == 0.f) { return 1.f; }

        const float result = vector.x() / _sinTheta;

        assert(result >= -1.f);
        assert(result <= 1.f);

        return result;
    }

    float cos2Phi(const Vector3 &vector)
    {
        return cosPhi(vector) * cosPhi(vector);
    }

    float sinPhi(const Vector3 &vector)
    {
        const float _sinTheta = sinTheta(vector);
        if (_sinTheta == 0.f) { return 0.f; }

        const float result = vector.y() / _sinTheta;

        assert(result >= -1.f);
        assert(result <= 1.f);

        return result;
    }

    float sin2Phi(const Vector3 &vector)
    {
        return sinPhi(vector) * sinPhi(vector);
    }
};
