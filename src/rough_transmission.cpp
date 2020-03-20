#include "rough_transmission.h"

#include "fresnel.h"
#include "monte_carlo.h"
#include "tangent_frame.h"
#include "transform.h"
#include "util.h"

#include <cmath>
#include <iostream>

Color RoughTransmission::f(
    const Intersection &intersection,
    const Vector3 &wiWorld,
    float *pdf
) const
{
    const Vector3 localWo = intersection.worldToTangent.apply(intersection.woWorld).normalized();
    const Vector3 localWi = intersection.worldToTangent.apply(wiWorld).normalized();

    bool isWoFrontside = localWo.y() >= 0.f;
    bool isWiFrontside = localWi.y() >= 0.f;

    if (isWoFrontside == isWiFrontside) {
        *pdf = 0.f;
        return Color(0.f);
    }

    float etaIncident = 1.f;
    float etaTransmitted = m_ior;

    if (!isWiFrontside) {
        std::swap(etaIncident, etaTransmitted);
    }

    const float cosThetaO = TangentFrame::absCosTheta(localWo);
    const float cosThetaI = TangentFrame::absCosTheta(localWi);
    const Vector3 wh = (localWo + localWi).normalized();

    *pdf = m_distributionPtr->pdf(wh) / (4.f * localWo.dot(wh));

    if (cosThetaO == 0.f || cosThetaI == 0.f) { return Color(0.f); }
    if (wh.isZero()) { return Color(0.f); }

    float cosThetaIncident = util::clampClose(localWi.dot(wh), 0.f, 1.f);
    float fresnel(Fresnel::dielectricReflectance(cosThetaIncident, 1.f, 1.5f));
    float distribution = m_distributionPtr->D(wh);
    float masking = m_distributionPtr->G(localWo, localWi);
    Color albedo(1.f);

    // std::cout << "D: " << distribution << std::endl;
    // std::cout << "G: " << masking << std::endl;
    // std::cout << "F: " << fresnel << std::endl;
    // std::cout << "cos's: " << cosThetaI << " " << cosThetaO << std::endl;

    const float dotProducts = (localWi.absDot(wh) * localWo.absDot(wh)) / (cosThetaO + cosThetaI);
    const float numerator = etaTransmitted * (1.f - fresnel) * distribution * masking;
    const float denominator = util::square(
        etaIncident * localWi.absDot(wh) + etaTransmitted * localWo.absDot(wh)
    );

    const Color value = albedo * numerator / denominator;

    // std::cout << "value: " << value << std::endl;

    return value;
}

BSDFSample RoughTransmission::sample(
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

