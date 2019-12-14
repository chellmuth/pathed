#include "microfacet.h"

#include "beckmann.h"
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

    const float cosThetaO = fabsf(TangentFrame::cosTheta(wo));
    const float cosThetaI = fabsf(TangentFrame::cosTheta(wi));
    const Vector3 wh = (wo + wi).normalized();

    *pdf = beckmannPDF(m_alpha, wh);

    if (cosThetaO == 0.f || cosThetaI == 0.f) { return Color(0.f); }
    if (wh.x() == 0.f || wh.y() == 0.f || wh.z() == 0.f) { return Color(0.f); }

    float cosThetaIncident = util::clampClose(wi.dot(wh), 0.f, 1.f);
    float fresnel(Fresnel::dielectricReflectance(cosThetaIncident, 1.f, 1.4f));
    float distribution = beckmannD(m_alpha, wh);
    float masking = beckmannG(m_alpha, m_alpha, wo, wi);
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
    const Vector3 wo = intersection.worldToTangent.apply(intersection.woWorld);
    const Vector3 wh = beckmannSampleWh(m_alpha, wo, random);
    const Vector3 wi = wo.reflect(wh);

    const Vector3 wiWorld = intersection.tangentToWorld.apply(wi);

    BSDFSample sample = {
        .wiWorld = wiWorld,
        .pdf = beckmannPDF(m_alpha, wh),
        .throughput = Material::f(intersection, wiWorld),
        .material = this
    };

    return sample;
}

