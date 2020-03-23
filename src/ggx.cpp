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
    const float xi1 = random.next();
    const float xi2 = random.next();

    const float numerator = m_alpha * std::sqrt(xi1);
    const float denominator = std::sqrt(1.f - xi1);

    const float theta = std::atan(numerator / denominator);
    const float phi = M_TWO_PI * xi2;

    const Vector3 cartesian = sphericalToCartesian(phi, theta);
    return cartesian;
}

float GGX::pdf(const Vector3 &wh) const
{
    return D(wh) * TangentFrame::absCosTheta(wh);
}

float GGX::D(const Vector3 &wh) const
{
    const float alpha2 = m_alpha * m_alpha;

    const float cos2Theta = TangentFrame::cos2Theta(wh);
    const float cos4Theta = cos2Theta * cos2Theta;

    const float tan2Theta = TangentFrame::tan2Theta(wh);
    if (std::isinf(tan2Theta)) { return 0.f; }

    const float sum = alpha2 + tan2Theta;
    const float denominator = M_PI * cos4Theta * sum * sum;

    return alpha2 / denominator;
}

float GGX::G1(const Vector3 &v) const
{
    const float tan2Theta = TangentFrame::tan2Theta(v);
    if (std::isinf(tan2Theta)) { return 0.f; }

    const float alpha2 = m_alpha * m_alpha;
    const float sqrtTerm = (1 + alpha2 * tan2Theta);

    return 2.f / (1 + std::sqrt(sqrtTerm));
}

float GGX::G(const Vector3 &wo, const Vector3 &wi, const Vector3 &wh) const
{
    return G1(wo) * G1(wi);
}
