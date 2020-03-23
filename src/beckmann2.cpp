#include "beckmann2.h"

#include "coordinate.h"
#include "tangent_frame.h"
#include "trig.h"
#include "util.h"

#include <cmath>

Beckmann2::Beckmann2(float alpha)
    : m_alpha(alpha)
{}

Vector3 Beckmann2::sampleWh(const Vector3 &wo, RandomGenerator &random) const
{
    const float phi = random.next() * M_PI * 2.f;

    const float alpha2 = m_alpha * m_alpha;
    const float sqrtTerm = (-alpha2 * std::log(1 - random.next()));
    const float theta = std::atan(std::sqrt(sqrtTerm));

    const Vector3 sample = sphericalToCartesian(phi, theta);
    return sample;
}

float Beckmann2::pdf(const Vector3 &wh) const
{
    return D(wh) * std::abs(TangentFrame::cosTheta(wh));
}

float Beckmann2::D(const Vector3 &wh) const
{
    const float alpha2 = m_alpha * m_alpha;

    const float cos2Theta = TangentFrame::cos2Theta(wh);
    const float cos4Theta = cos2Theta * cos2Theta;

    const float tan2Theta = TangentFrame::tan2Theta(wh);
    if (std::isinf(tan2Theta)) { return 0.f; }

    const float numerator = std::exp(-tan2Theta / alpha2);
    const float denominator = M_PI * alpha2 * cos4Theta;

    return numerator / denominator;
}

float Beckmann2::G1(const Vector3 &v) const
{
    const float a = 1.f / (m_alpha * TangentFrame::absTanTheta(v));

    if (a >= 1.6f) { return 1.f; }

    const float numerator = 3.535 * a + 2.181 * a * a;
    const float denominator = 1 + 2.276 * a + 2.577 * a * a;

    return numerator / denominator;
}

float Beckmann2::G(const Vector3 &wo, const Vector3 &wi, const Vector3 &wh) const
{
    if (util::sign(wo.dot(wh)) != util::sign(TangentFrame::cosTheta(wo))) {
        return 0.f;
    }
    if (util::sign(wi.dot(wh)) != util::sign(TangentFrame::cosTheta(wi))) {
        return 0.f;
    }
    return G1(wi) * G1(wo);
}
