#pragma once

#include "vector.h"

#include <cmath>


namespace TangentFrame {
    float cosTheta(const Vector3 &vector) const
    {
        return vector.y();
    }

    float cos2Theta(const Vector3 &vector) const
    {
        return vector.y() * vector.y();
    }

    float sinTheta(const Vector3 &vector) const
    {
        return fsqrtf(1.f - cos2Theta(vector));
    }
};
