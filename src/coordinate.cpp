#include "coordinate.h"

#include "util.h"

#include <cmath>

void cartesianToSpherical(Vector3 cartesian, float *phi, float *theta)
{
    *phi = atan2f(cartesian.z(), cartesian.x());
    if (*phi < 0.f) {
        *phi += 2 * M_PI;
    }
    if (*phi == M_TWO_PI) {
        *phi = 0;
    }

    *theta = acosf(util::clampClose(cartesian.y(), -1.f, 1.f));
}

Vector3 sphericalToCartesian(float phi, float theta)
{
    return sphericalToCartesian(phi, cosf(theta), sinf(theta));
}

Vector3 sphericalToCartesian(float phi, float cosTheta, float sinTheta)
{
    const float y = cosTheta;
    const float x = sinTheta * cosf(phi);
    const float z = sinTheta * sinf(phi);

    return Vector3(x, y, z);
}
