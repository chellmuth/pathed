#include "microfacet.h"

#include "fresnel.h"
#include "monte_carlo.h"
#include "tangent_frame.h"
#include "transform.h"
#include "util.h"

#include <cmath>
#include <iostream>

Color Microfacet::f(
    const Intersection &intersection,
    const Vector3 &wiWorld,
    float *pdf
) const
{
    const auto [wi, wo] = buildLocalWs(intersection, wiWorld);

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

    *pdf = m_distributionPtr->pdf(wi, wh) / (4.f * wo.absDot(wh));

    if (cosThetaO == 0.f || cosThetaI == 0.f) { return Color(0.f); }
    if (wh.isZero()) { return Color(0.f); }

    float cosThetaIncident = util::clampClose(wi.dot(wh), 0.f, 1.f);
    float fresnel(Fresnel::dielectricReflectance(cosThetaIncident, 1.f, 1.5f));
    float distribution = m_distributionPtr->D(wi, wh);
    float masking = m_distributionPtr->G(wi, wo, wh);
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

BSDFSample Microfacet::sample(
    const Intersection &intersection,
    RandomGenerator &random
) const
{
    const Vector3 wi = buildLocalWi(intersection);
    const Vector3 wh = m_distributionPtr->sampleWh(wi, random);
    const Vector3 wo = wi.reflect(wh);

    const Vector3 woWorld = intersection.tangentToWorld.apply(wo);

    BSDFSample sample = {
        .wiWorld = woWorld,
        .pdf = m_distributionPtr->pdf(wi, wh) / (4.f * wo.absDot(wh)),
        .throughput = Material::f(intersection, woWorld),
        .material = this
    };

    return sample;
}

