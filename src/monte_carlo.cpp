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

float UniformHemispherePdf(const Vector3 &v)
{
    return INV_TWO_PI;
}

Vector3 CosineSampleHemisphere(RandomGenerator &random)
{
    const float xi1 = random.next();
    const float r = sqrtf(xi1);
    const float phi = 2 * M_PI * random.next();

    const float x = r * cosf(phi);
    const float z = r * sinf(phi);

    const float y = sqrtf(1.f - xi1); // a^2 + b^2 = c^2

    return Vector3(x, y, z);
}

float CosineHemispherePdf(const Vector3 &v)
{
    return v.y() * INV_PI;
}

Vector3 UniformSampleSphere(RandomGenerator &random)
{
    float z = random.next() * 2.f - 1;
    float r = sqrtf(fmaxf(0.f, 1.f - z*z));
    float phi = 2 * M_PI * random.next();
    float x = r * cosf(phi);
    float y = r * sinf(phi);

    return Vector3(x, z, y);
}

float UniformSampleSpherePDF(const Vector3 &v)
{
    return INV_TWO_PI / 2.f;
}
