#include "rough_transmission.h"

#include "fresnel.h"
#include "logger.h"
#include "monte_carlo.h"
#include "tangent_frame.h"
#include "transform.h"
#include "util.h"

#include <cmath>

static float reflectJacobian(float woDotWh)
{
    return 1.f / (4.f * woDotWh);
}

static float refractJacobian(
    float etaIncident,
    float etaTransmitted,
    float wiDotWh,
    float woDotWh
) {
    const float numerator = util::square(etaTransmitted) * woDotWh;
    const float denominator = util::square(
        etaIncident * wiDotWh + etaTransmitted * woDotWh
    );
    return numerator / denominator;
}

Color RoughTransmission::f(
    const Intersection &intersection,
    const Vector3 &wiWorld,
    float *pdf
) const
{
    const Vector3 localWo = intersection.worldToTangent.apply(intersection.woWorld).normalized();
    const Vector3 localWi = intersection.worldToTangent.apply(wiWorld).normalized();

    float etaIncident = 1.f;
    float etaTransmitted = m_ior;

    const bool isWiFrontside = localWi.y() >= 0.f;
    const bool isWoFrontside = localWo.y() >= 0.f;
    if (!isWoFrontside) {
        std::swap(etaIncident, etaTransmitted);
    }

    const bool isReflect = isWiFrontside == isWoFrontside;

    const float cosThetaO = TangentFrame::absCosTheta(localWo);
    const float cosThetaI = TangentFrame::absCosTheta(localWi);
    const Vector3 wh = (localWo + localWi).normalized();

    const float cosThetaIncident = util::clampClose(localWi.dot(wh), 0.f, 1.f);
    const float fresnel = Fresnel::dielectricReflectance(
        cosThetaIncident,
        etaIncident,
        etaTransmitted
    );

    const float wiDotWh = localWi.absDot(wh);
    const float woDotWh = localWo.absDot(wh);

    if (cosThetaO == 0.f || cosThetaI == 0.f) { return Color(0.f); }
    if (wh.isZero()) { return Color(0.f); }

    const float distribution = m_distributionPtr->D(wh);
    const float masking = m_distributionPtr->G(localWo, localWi);

    const Color albedo(1.f);

    if (isReflect) {
        *pdf = m_distributionPtr->pdf(wh) * reflectJacobian(woDotWh);

        const Color value = albedo * distribution * masking * fresnel
            / (4 * cosThetaI * cosThetaO);

        return value;

    } else {
        *pdf = m_distributionPtr->pdf(wh) * refractJacobian(
            etaIncident,
            etaTransmitted,
            wiDotWh,
            woDotWh
        );

        const float dotProducts = (wiDotWh * woDotWh) / (cosThetaO + cosThetaI);
        const float numerator = etaTransmitted * (1.f - fresnel) * distribution * masking;
        const float denominator = util::square(
            etaIncident * wiDotWh + etaTransmitted * woDotWh
        );

        const Color value = albedo * numerator / denominator;

        if (value.r() < 0.f || value.g() < 0.f || value.b() < 0.f) {
            Logger::cout << value << std::endl;
        }

        // std::cout << "value: " << value << std::endl;

        return value;
    }
}

BSDFSample RoughTransmission::sample(
    const Intersection &intersection,
    RandomGenerator &random
) const
{
    // const Vector3 localWo = intersection.worldToTangent.apply(intersection.woWorld);
    // Vector3 localWi(0.f);

    // float etaIncident = 1.f;
    // float etaTransmitted = m_ior;

    // if (localWo.y() < 0.f) {
    //     std::swap(etaIncident, etaTransmitted);
    // }

    // const bool doesRefract = Snell::refract(
    //     localWo,
    //     &localWi,
    //     etaIncident,
    //     etaTransmitted
    // );

    // const float fresnelReflectance = Fresnel::dielectricReflectance(
    //     TangentFrame::absCosTheta(localWo),
    //     etaIncident, etaTransmitted
    // );

    // const Vector3 wh = m_distributionPtr->sampleWh(wo, random);

    // const Vector3 wiWorld = intersection.tangentToWorld.apply(wi);

    // const BSDFSample sample = {
    //     .wiWorld = wiWorld,
    //     .pdf = m_distributionPtr->pdf(wh) / (4.f * wo.dot(wh)),
    //     .throughput = Material::f(intersection, wiWorld),
    //     .material = this
    // };

    const BSDFSample sample = {
        .wiWorld = Vector3(0.f, 0.f, 0.f),
        .pdf = 0.f,
        .throughput = Color(0.f),
        .material = this
    };

    return sample;
}
