#include "rough_transmission.h"

#include "fresnel.h"
#include "logger.h"
#include "monte_carlo.h"
#include "snell.h"
#include "tangent_frame.h"
#include "transform.h"
#include "util.h"

#include <cmath>

static float reflectJacobian(float woAbsDotWh)
{
    return 1.f / (4.f * woAbsDotWh);
}

static float refractJacobian(
    float etaIncident,
    float etaTransmitted,
    float wiDotWh,
    float woDotWh
) {
    const float numerator = util::square(etaTransmitted) * std::abs(woDotWh);
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
    const auto [localWi, localWo] = buildLocalWs(intersection, wiWorld);

    float etaIncident = 1.f;
    float etaTransmitted = m_ior;

    const bool isWiFrontside = localWi.y() >= 0.f;
    const bool isWoFrontside = localWo.y() >= 0.f;
    if (!isWiFrontside) {
        std::swap(etaIncident, etaTransmitted);
    }

    const bool isReflect = isWiFrontside == isWoFrontside;

    const float cosThetaI = TangentFrame::absCosTheta(localWi);
    const float cosThetaO = TangentFrame::absCosTheta(localWo);
    const Vector3 wh = Snell::computeHalfVector(
        localWi,
        localWo,
        etaIncident,
        etaTransmitted,
        isReflect
    );

    const float wiDotWh = localWi.dot(wh);
    const float wiAbsDotWh = util::clamp(localWi.absDot(wh), 0.f, 1.);
    const float fresnel = Fresnel::dielectricReflectanceWalter(
        localWi,
        wh,
        etaIncident,
        etaTransmitted
    );

    const float woDotWh = localWo.dot(wh);
    const float woAbsDotWh = localWo.absDot(wh);

    if (cosThetaI == 0.f || cosThetaO == 0.f) { return Color(0.f); }
    if (wh.isZero()) { return Color(0.f); }

    const float distribution = m_distributionPtr->D(wh);
    const float masking = m_distributionPtr->G(localWi, localWo, wh);

    const Color albedo(1.f);

    if (isReflect) {
        *pdf = m_distributionPtr->pdf(wh) * reflectJacobian(woAbsDotWh);

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

        const float dotProducts = (wiAbsDotWh * woAbsDotWh) / (cosThetaI * cosThetaO);
        const float numerator = util::square(etaTransmitted)
            * (1.f - fresnel)
            * distribution
            * masking;
        const float denominator = util::square(
            etaIncident * wiDotWh + etaTransmitted * woDotWh
        );

        if (denominator == 0.f) { return Color(0.f); }

        // PBRT page 961 "Non-symmetry Due to Refraction"
        // Always incident / transmitted because we swap at top of
        // function if we're going inside-out
        const float nonSymmetricEtaCorrection = util::square(
            etaIncident / etaTransmitted
        );

        const Color value = albedo
            * dotProducts
            * (numerator / denominator)
            * nonSymmetricEtaCorrection;

        return value;
    }
}

BSDFSample RoughTransmission::sample(
    const Intersection &intersection,
    RandomGenerator &random
) const
{
    const Vector3 localWi = buildLocalWi(intersection);
    const Vector3 wh = m_distributionPtr->sampleWh(localWi, random);

    float etaIncident = 1.f;
    float etaTransmitted = m_ior;

    if (localWi.y() < 0.f) {
        std::swap(etaIncident, etaTransmitted);
    }

    const float wiDotWh = localWi.dot(wh);
    const float wiAbsDotWh = util::clamp(localWi.absDot(wh), 0.f, 1.f);

    const float fresnelReflectance = Fresnel::dielectricReflectanceWalter(
        localWi,
        wh,
        etaIncident, etaTransmitted
    );

    if (random.next() < fresnelReflectance) {
        const Vector3 localWo = localWi.reflect(wh);
        const Vector3 woWorld = intersection.tangentToWorld.apply(localWo);

        const float woAbsDotWh = localWo.absDot(wh);
        const float pdf = m_distributionPtr->pdf(wh)
            * reflectJacobian(woAbsDotWh)
            * fresnelReflectance;

        const Color throughput = Material::f(intersection, woWorld);

        const BSDFSample sample = {
            .wiWorld = woWorld,
            .pdf = pdf,
            .throughput = throughput,
            .material = this
        };

        return sample;
    } else {
        Vector3 localWo(0.f);

        const bool doesRefract = Snell::refract(
            localWi,
            &localWo,
            wh,
            etaIncident,
            etaTransmitted
        );
        assert(doesRefract);

        const Vector3 woWorld = intersection.tangentToWorld.apply(localWo);

        const float fresnelTransmittance = 1.f - fresnelReflectance;
        const float woDotWh = localWo.dot(wh);

        const float jacobian = refractJacobian(
            etaIncident,
            etaTransmitted,
            wiDotWh,
            woDotWh
        );

        const Color throughput = Material::f(intersection, woWorld);
        const float pdf = m_distributionPtr->pdf(wh)
            * jacobian
            * fresnelTransmittance;

        const BSDFSample sample = {
            .wiWorld = woWorld,
            .pdf = pdf,
            .throughput = throughput,
            .material = this
        };

        return sample;
    }
}
