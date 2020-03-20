#include "rough_refraction.h"

#include "fresnel.h"
#include "monte_carlo.h"
#include "tangent_frame.h"
#include "transform.h"
#include "util.h"

#include <cmath>
#include <iostream>

Color RoughRefraction::f(
    const Intersection &intersection,
    const Vector3 &wiWorld,
    float *pdf
) const
{
    const Vector3 wo = intersection.worldToTangent.apply(intersection.woWorld).normalized();
    const Vector3 wi = intersection.worldToTangent.apply(wiWorld).normalized();

    if (intersection.woWorld.dot(intersection.shadingNormal) < 0.f) {
        *pdf = 0.f;
        return Color(0.f);
    }

    if (wiWorld.dot(intersection.shadingNormal) < 0.f) {
        *pdf = 0.f;
        return Color(0.f);
    }

    const float cosThetaO = TangentFrame::absCosTheta(wo);
    const float cosThetaI = TangentFrame::absCosTheta(wi);
    const Vector3 wh = (wo + wi).normalized();

    *pdf = m_distributionPtr->pdf(wh) / (4.f * wo.dot(wh));

    if (cosThetaO == 0.f || cosThetaI == 0.f) { return Color(0.f); }
    if (wh.isZero()) { return Color(0.f); }

    float cosThetaIncident = util::clampClose(wi.dot(wh), 0.f, 1.f);
    float fresnel(Fresnel::dielectricReflectance(cosThetaIncident, 1.f, 1.5f));
    float distribution = m_distributionPtr->D(wh);
    float masking = m_distributionPtr->G(wo, wi);
    Color albedo(1.f);

    // std::cout << "D: " << distribution << std::endl;
    // std::cout << "G: " << masking << std::endl;
    // std::cout << "F: " << fresnel << std::endl;
    // std::cout << "cos's: " << cosThetaI << " " << cosThetaO << std::endl;

    const Color value = albedo * distribution * masking * fresnel
        / (4 * cosThetaI * cosThetaO);

    // std::cout << "value: " << value << std::endl;

    return value;
}

BSDFSample RoughRefraction::sample(
    const Intersection &intersection,
    RandomGenerator &random
) const
{
    const Vector3 wo = intersection.worldToTangent.apply(intersection.woWorld);
    const Vector3 wh = m_distributionPtr->sampleWh(wo, random);
    const Vector3 wi = wo.reflect(wh);

    const Vector3 wiWorld = intersection.tangentToWorld.apply(wi);

    BSDFSample sample = {
        .wiWorld = wiWorld,
        .pdf = m_distributionPtr->pdf(wh) / (4.f * wo.dot(wh)),
        .throughput = Material::f(intersection, wiWorld),
        .material = this
    };

    return sample;
}

