#include "rough_dielectric.h"

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

Color RoughDielectric::f(
    const Intersection &intersection,
    const Vector3 &wiWorld,
    float *pdf
) const
{
    const auto [localWi, localWo] = buildLocalWs(intersection, wiWorld);

    const float etaExternal = 1.f;
    const float etaInternal = m_ior;

    float etaIncident = etaExternal;
    float etaTransmitted = etaInternal;

    const bool isWiFrontside = TangentFrame::cosTheta(localWi) >= 0.f;
    const bool isWoFrontside = TangentFrame::cosTheta(localWo) >= 0.f;

    if (!isWiFrontside) {
        std::swap(etaIncident, etaTransmitted);
    }

    const bool isReflect = isWiFrontside == isWoFrontside;

    const float cosThetaI = TangentFrame::absCosTheta(localWi);
    const float cosThetaO = TangentFrame::absCosTheta(localWo);
    Vector3 wh = Snell::computeHalfVector(
        localWi,
        localWo,
        etaIncident,
        etaTransmitted,
        isReflect
    );

    if (TangentFrame::cosTheta(wh) < 0.f) {
        wh = wh.negate();
    }

    const float wiDotWh = localWi.dot(wh);

    const float wiAbsDotWh = util::clamp(localWi.absDot(wh), 0.f, 1.);
    const float fresnel = Fresnel::dielectricReflectanceWalter(
        localWi,
        wh,
        etaExternal,
        etaInternal
    );

    const float woDotWh = localWo.dot(wh);
    const float woAbsDotWh = localWo.absDot(wh);

    if (cosThetaI == 0.f || cosThetaO == 0.f) { return Color(0.f); }
    if (wh.isZero()) { return Color(0.f); }

    const float distribution = m_distributionPtr->D(localWi, wh);
    const float masking = m_distributionPtr->G(localWi, localWo, wh);

    if (distribution < 1e-6) {
        *pdf = 0;
        return Color(0.f);
    }

    const Color albedo(1.f);

    if (isReflect) {
        const float distributionPDF = m_distributionPtr->pdf(localWi, wh);
        const float jacobian = reflectJacobian(woAbsDotWh);

        *pdf = distributionPDF * jacobian * fresnel;

        const Color value = albedo * distribution * masking * fresnel
            / (4 * cosThetaI * cosThetaO);

        return value;

    } else {
        if (fresnel == 1.f) {
            *pdf = 0.f;
            return Color(0.f);
        }

        const float distributionPDF = m_distributionPtr->pdf(localWi, wh);
        const float jacobian = refractJacobian(
            etaIncident,
            etaTransmitted,
            wiDotWh,
            woDotWh
        );

        *pdf = distributionPDF * jacobian * (1.f - fresnel);

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

static BSDFSample invalidSample(const Material *material) {
    const BSDFSample sample = {
        .wiWorld = Vector3(0.f, 0.f, 0.f),
        .pdf = 0.f,
        .throughput = Color(0.f),
        .material = material
    };
    return sample;
}

BSDFSample RoughDielectric::sample(
    const Intersection &intersection,
    RandomGenerator &random
) const
{
    const Vector3 localWi = buildLocalWi(intersection);
    const float cosThetaI = TangentFrame::cosTheta(localWi);

    // Note: doesn't use localWi
    Vector3 wh = m_distributionPtr->sampleWh(localWi, random);

    const float etaExternal = 1.f;
    const float etaInternal = m_ior;

    float etaIncident = etaExternal;
    float etaTransmitted = etaInternal;

    if (TangentFrame::cosTheta(localWi) < 0.f) {
        std::swap(etaIncident, etaTransmitted);
    }

    const float wiDotWh = localWi.dot(wh);
    const float wiAbsDotWh = util::clamp(localWi.absDot(wh), 0.f, 1.f);

    const float fresnelReflectance = Fresnel::dielectricReflectanceWalter(
        localWi, wh,
        etaExternal, etaInternal
    );

    if (random.next() < fresnelReflectance) {
        const Vector3 localWo = localWi.reflect(wh);
        const Vector3 woWorld = intersection.tangentToWorld.apply(localWo);

        if (TangentFrame::cosTheta(localWo) * cosThetaI < 0.f) {
            return invalidSample(this);
        }

        const float woAbsDotWh = localWo.absDot(wh);
        const float pdf = m_distributionPtr->pdf(localWi, wh)
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
        const float cosThetaTransmitted = Snell::cosThetaTransmitted(
            wiDotWh,
            etaExternal,
            etaInternal
        );
        const Vector3 localWo = Snell::refract(
            localWi,
            wh,
            cosThetaTransmitted,
            etaExternal,
            etaInternal
        );

        if (TangentFrame::cosTheta(localWo) * cosThetaI > 0.f) {
            return invalidSample(this);
        }

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
        const float pdf = m_distributionPtr->pdf(localWi, wh)
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
