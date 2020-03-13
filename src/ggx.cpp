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
    const float alpha2 = m_alpha * m_alpha;

    const float cos2Theta = TangentFrame::cos2Theta(wh);
    const float cos4Theta = cos2Theta * cos2Theta;

    const float tan2Theta = TangentFrame::tan2Theta(wh);
    if (std::isinf(tan2Theta)) { return 0.f; }

    const float sum = alpha2 + tan2Theta;
    const float denominator = M_PI * cos4Theta * sum * sum;

    return alpha2 / denominator;
}

float GGX::G(const Vector3 &wo, const Vector3 &wi) const
{
    const float tan2Theta = TangentFrame::tan2Theta(wo);
    if (std::isinf(tan2Theta)) { return 0.f; }

    const float alpha2 = m_alpha * m_alpha;
    const float sqrtTerm = (1 + alpha2 * tan2Theta);

    return 2.f / (1 + std::sqrt(sqrtTerm));
}
