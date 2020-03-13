#include "ggx.h"

#include "coordinate.h"
#include "tangent_frame.h"
#include "trig.h"

#include <cmath>

GGX::GGX(float alpha)
    : m_alpha(alpha)
{}

Vector3 GGX::sampleWh(const Vector3 &wo, RandomGenerator &random) const
{
    return Vector3(0.f, 0.f, 0.f);
}

float GGX::pdf(const Vector3 &wh) const
{
    return 0.f;
}

float GGX::D(const Vector3 &wh) const
{
    return 0.f;
}

float GGX::G(const Vector3 &wo, const Vector3 &wi) const
{
    return 0.f;
}
