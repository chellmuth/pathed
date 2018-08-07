#include "monte_carlo.h"

#include <math.h>
#include <random>

#include <iostream>

#include "util.h"

Vector3 UniformSampleHemisphere()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);

    float z = dis(gen);
    float r = sqrtf(fmaxf(0.f, 1.f - z*z));
    float phi = 2 * M_PI * dis(gen);
    float x = r * cosf(phi);
    float y = r * sinf(phi);

    return Vector3(x, z, y);
}

float UniformHemispherePdf()
{
    return INV_TWOPI;
}
