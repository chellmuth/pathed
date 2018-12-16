#include "monte_carlo.h"

#include <math.h>
#include <iostream>

#include "util.h"

Vector3 UniformSampleHemisphere(RandomGenerator &random)
{
    float z = random.next();
    float r = sqrtf(fmaxf(0.f, 1.f - z*z));
    float phi = 2 * M_PI * random.next();
    float x = r * cosf(phi);
    float y = r * sinf(phi);

    return Vector3(x, z, y);
}

float UniformHemispherePdf()
{
    return INV_TWO_PI;
}
