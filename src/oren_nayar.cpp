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
    const Vector3 &wi,
    float *pdf
) const
{
    if (intersection.normal.dot(intersection.wo) < 0.f) {
        *pdf = 1.f;
        return Color(0.f);
    }

    if (intersection.shadingNormal.dot(intersection.wo) < 0.f) {
        *pdf = 1.f;
        return Color(0.f);
    }

    const Transform worldToTangent = worldSpaceToNormal(
        intersection.shadingNormal,
        intersection.wo
    );

    const Vector3 localWo = worldToTangent.apply(intersection.wo);
    const Vector3 localWi = worldToTangent.apply(wi);

    if (localWo.y() < 0.f) {
        *pdf = 1.f;
        return Color(0.f);
    }

    if (localWi.y() < 0.f) {
        *pdf = 1.f;
        // std::cout << localWo.toString()
        //           << " " << intersection.wo.dot(intersection.shadingNormal)
        //           << " " << intersection.wo.dot(intersection.normal) << std::endl;
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

    if (throughput < 0.f || throughput * cosf(thetaO) > 1.f) {
        std::cout << m_A << " " << m_B << " " << tanf(beta) << std::endl;
        std::cout << throughput << " " << throughput * cosf(thetaO) << std::endl;
    }


    Color albedo = Color(0.8f, 0.5f, 0.4f);
    return albedo * throughput;
}

BSDFSample OrenNayar::sample(
    const Intersection &intersection,
    RandomGenerator &random
) const
{
    Transform tangentToWorld = normalToWorldSpace(
        intersection.shadingNormal,
        intersection.wo
    );

    Vector3 localSample = CosineSampleHemisphere(random);
    Vector3 worldSample = tangentToWorld.apply(localSample);

    BSDFSample sample = {
        .wi = worldSample,
        .pdf = CosineHemispherePdf(localSample),
        .throughput = Material::f(intersection, worldSample)
    };

    return sample;
}
