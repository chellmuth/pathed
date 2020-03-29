#include "beckmann.h"

#include "coordinate.h"
#include "tangent_frame.h"
#include "trig.h"

#include <cmath>

Beckmann::Beckmann(float alpha)
    : m_alpha(alpha)
{}

static Vector3 cartestianFromTan2Theta(float tan2Theta, float phi)
{
    const float cosTheta = 1.f / sqrtf(1.f + tan2Theta);
    const float sinTheta = Trig::sinFromCos(cosTheta);

    return sphericalToCartesian(phi, cosTheta, sinTheta);
}

static float sampleTan2Theta(float alpha, const Vector3 &wi, RandomGenerator &random)
{
    const float xi = random.next();

    float logXi = std::log(xi);
    if (std::isinf(logXi)) { logXi = 0.f; }

    return -alpha * alpha * logXi;
}

Vector3 Beckmann::sampleWh(const Vector3 &wi, RandomGenerator &random) const
{
    const float phi = random.next() * M_PI * 2.f;
    const float tan2Theta = sampleTan2Theta(m_alpha, wi, random);

    const Vector3 sample = cartestianFromTan2Theta(tan2Theta, phi);
    return sample;
}

float Beckmann::pdf(const Vector3 &wi, const Vector3 &wh) const
{
    return D(wi, wh) * std::abs(TangentFrame::cosTheta(wh));
}

float Beckmann::D(const Vector3 &wi, const Vector3 &wh) const
{
    const float tan2Theta = TangentFrame::tan2Theta(wh);
    if (std::isinf(tan2Theta)) { return 0.f; }

    const float cos2Theta = TangentFrame::cos2Theta(wh);
    const float cos4Theta = cos2Theta * cos2Theta;
    const float alpha2 = m_alpha * m_alpha;

    const float numerator = std::exp(
        -tan2Theta * (
            (TangentFrame::cos2Phi(wh) / alpha2)
            + (TangentFrame::sin2Phi(wh) / alpha2)
        )
    );
    const float denominator = M_PI * alpha2 * cos4Theta;

    return numerator / denominator;
}

static float lambda(float alphaX, float alphaY, const Vector3 &w)
{
    const float absTanTheta = std::abs(TangentFrame::tanTheta(w));
    if (std::isinf(absTanTheta)) { return 0.f; }

    const float alpha = sqrtf(
        TangentFrame::cos2Phi(w) * alphaX * alphaX
        + TangentFrame::sin2Phi(w) * alphaY * alphaY
    );

    const float a = 1.f / (alpha * absTanTheta);

    if (a >= 1.6f) { return 0.f; }

    return (1 - 1.259f * a + 0.396f * a * a)
        / (3.535f * a + 2.181f * a * a);
}

float Beckmann::G(const Vector3 &wi, const Vector3 &wo, const Vector3 &wh) const
{
    const float alphaX = m_alpha;
    const float alphaY = m_alpha;

    return 1.f / (1.f + lambda(alphaX, alphaY, wo) + lambda(alphaX, alphaY, wi));
}
