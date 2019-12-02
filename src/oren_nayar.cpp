#include "oren_nayar.h"

#include "coordinate.h"
#include "monte_carlo.h"
#include "transform.h"
#include "util.h"

#include <algorithm>
#include <cmath>
#include <iostream>

OrenNayar::OrenNayar()
    : Material(0.f)
{
    const float sigma = 0.8f;
    const float sigma2 = sigma * sigma;

    m_A = 1.f - (sigma2 / (2.f * (sigma2 + 0.33f)));
    m_B = (0.45f * sigma2) / (sigma2 + 0.09f);
}

Color OrenNayar::f(
    const Intersection &intersection,
    const Vector3 &wo,
    float *pdf
) const
{
    if (intersection.normal.dot(-intersection.wi) < 0.f) {
        *pdf = 1.f;
        return Color(0.f);
    }

    if (intersection.shadingNormal.dot(-intersection.wi) < 0.f) {
        *pdf = 1.f;
        return Color(0.f);
    }

    // Wo and Wi are backwards...

    const Transform worldToTangent = worldSpaceToNormal(
        intersection.shadingNormal,
        intersection.wi
    );

    const Vector3 localWi = worldToTangent.apply(-intersection.wi);
    const Vector3 localWo = worldToTangent.apply(wo);

    if (localWo.y() < 0.f) {
        *pdf = 1.f;
        return Color(0.f);
    }

    if (localWi.y() < 0.f) {
        *pdf = 1.f;
        std::cout << localWi.toString()
                  << " " << (-intersection.wi).dot(intersection.shadingNormal)
                  << " " << (-intersection.wi).dot(intersection.normal) << std::endl;
        return Color(0.f);
    }

    float phiI, thetaI;
    float phiO, thetaO;

    cartesianToSpherical(localWi, &phiI, &thetaI);
    cartesianToSpherical(localWo, &phiO, &thetaO);

    const float alpha = std::max(thetaI, thetaO);
    const float beta = std::min(thetaI, thetaO);

    *pdf = CosineHemispherePdf(localWo);

    const float throughput = INV_PI * (
        m_A \
        + m_B * std::max(0.f, cosf(phiO - phiI)) \
            * sinf(alpha) \
            * tanf(beta));

    if (throughput < 0.f || throughput * cosf(thetaO) > 1.f) {
        std::cout << m_A << " " << m_B << " " << tanf(beta) << std::endl;
        std::cout << throughput << " " << throughput * cosf(thetaO) << std::endl;
    }
    return throughput;
}

BSDFSample OrenNayar::sample(
    const Intersection &intersection,
    RandomGenerator &random
) const
{
    Transform tangentToWorld = normalToWorldSpace(
        intersection.shadingNormal,
        intersection.wi
    );

    Vector3 localSample = CosineSampleHemisphere(random);
    Vector3 worldSample = tangentToWorld.apply(localSample);

    BSDFSample sample = {
        .wo = tangentToWorld.apply(localSample),
        .pdf = CosineHemispherePdf(localSample),
        .throughput = Material::f(intersection, worldSample)
    };

    return sample;
}
