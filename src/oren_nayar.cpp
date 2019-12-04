#include "oren_nayar.h"

#include "coordinate.h"
#include "monte_carlo.h"
#include "transform.h"
#include "util.h"

#include <algorithm>
#include <cmath>

OrenNayar::OrenNayar(Color diffuse, float sigma)
    : Material(0.f), m_diffuse(diffuse)
{
    const float sigma2 = sigma * sigma;

    m_A = 1.f - (sigma2 / (2.f * (sigma2 + 0.33f)));
    m_B = (0.45f * sigma2) / (sigma2 + 0.09f);
}

Color OrenNayar::f(
    const Intersection &intersection,
    const Vector3 &wi,
    float *pdf
) const
{
    if (intersection.normal.dot(intersection.woWorld) < 0.f) {
        *pdf = 1.f;
        return Color(0.f);
    }

    if (intersection.shadingNormal.dot(intersection.woWorld) < 0.f) {
        *pdf = 1.f;
        return Color(0.f);
    }

    const Transform worldToTangent = worldSpaceToNormal(
        intersection.shadingNormal,
        intersection.woWorld
    );

    const Vector3 localWo = worldToTangent.apply(intersection.woWorld).normalized();
    const Vector3 localWi = worldToTangent.apply(wi).normalized();

    if (localWo.y() < 0.f) {
        *pdf = 1.f;
        return Color(0.f);
    }

    if (localWi.y() < 0.f) {
        *pdf = 1.f;
        return Color(0.f);
    }

    float phiI, thetaI;
    float phiO, thetaO;

    cartesianToSpherical(localWi, &phiI, &thetaI);
    cartesianToSpherical(localWo, &phiO, &thetaO);

    const float alpha = std::max(thetaI, thetaO);
    const float beta = std::min(thetaI, thetaO);

    *pdf = CosineHemispherePdf(localWi);

    const float throughput = INV_PI * (
        m_A \
        + m_B * std::max(0.f, cosf(phiI - phiO)) \
            * sinf(alpha) \
            * tanf(beta));

    return m_diffuse * throughput;
}

BSDFSample OrenNayar::sample(
    const Intersection &intersection,
    RandomGenerator &random
) const
{
    Vector3 localSample = CosineSampleHemisphere(random);
    Vector3 worldSample = intersection.tangentToWorld.apply(localSample);

    BSDFSample sample = {
        .wi = worldSample,
        .pdf = CosineHemispherePdf(localSample),
        .throughput = Material::f(intersection, worldSample)
    };

    return sample;
}
