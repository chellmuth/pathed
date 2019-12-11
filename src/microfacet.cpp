#include "microfacet.h"

#include "beckmann.h"
#include "monte_carlo.h"
#include "tangent_frame.h"
#include "transform.h"

#include <cmath>

Color Microfacet::f(
    const Intersection &intersection,
    const Vector3 &wi,
    float *pdf
) const
{
    const Vector3 wo = intersection.worldToTangent.apply(intersection.woWorld);

    if (intersection.woWorld.dot(intersection.shadingNormal) < 0.f) {
        *pdf = 0.f;
        return Color(0.f);
    }

    *pdf = CosineHemispherePdf(wi);

    const float cosThetaO = fabsf(TangentFrame::cosTheta(wo));
    const float cosThetaI = fabsf(TangentFrame::cosTheta(wi));
    const Vector3 wh = (wo + wi).normalized();

    if (cosThetaO == 0.f || cosThetaI == 0.f) { return Color(0.f); }
    if (wh.x() == 0.f || wh.y() == 0.f || wh.z() == 0.f) { return Color(0.f); }

    Color fresnel(1.f);
    Color albedo(1.f);
    const float alpha = 1.f;

    return albedo * beckmannD(alpha, wh) * beckmannG(alpha, alpha, wi, wo) * fresnel
        / (4 * cosThetaI * cosThetaO);
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

