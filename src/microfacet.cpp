#include "microfacet.h"

#include "beckmann.h"
#include "fresnel.h"
#include "monte_carlo.h"
#include "tangent_frame.h"
#include "transform.h"

#include <cmath>
#include <iostream>

Color Microfacet::f(
    const Intersection &intersection,
    const Vector3 &wiWorld,
    float *pdf
) const
{
    const Vector3 wo = intersection.worldToTangent.apply(intersection.woWorld);
    const Vector3 wi = intersection.worldToTangent.apply(wiWorld).normalized();

    if (intersection.woWorld.dot(intersection.shadingNormal) < 0.f) {
        *pdf = 0.f;
        return Color(0.f);
    }

    if (wiWorld.dot(intersection.shadingNormal) < 0.f) {
        *pdf = 0.f;
        return Color(0.f);
    }

    *pdf = CosineHemispherePdf(wi);

    const float cosThetaO = fabsf(TangentFrame::cosTheta(wo));
    const float cosThetaI = fabsf(TangentFrame::cosTheta(wi));
    const Vector3 wh = (wo + wi).normalized();

    if (cosThetaO == 0.f || cosThetaI == 0.f) { return Color(0.f); }
    if (wh.x() == 0.f || wh.y() == 0.f || wh.z() == 0.f) { return Color(0.f); }

    float fresnel(Fresnel::dielectricReflectance(wi.dot(wh), 1.f, 1.4f));
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
    Vector3 localSample = CosineSampleHemisphere(random);
    Vector3 worldSample = intersection.tangentToWorld.apply(localSample);

    BSDFSample sample = {
        .wi = worldSample,
        .pdf = CosineHemispherePdf(localSample),
        .throughput = Material::f(intersection, worldSample)
    };

    return sample;
}

