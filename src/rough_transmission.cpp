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
    assert(woAbsDotWh >= 0.f);
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
    Logger::cout << "Rough eval w_h: " << wh.toString() << std::endl;

    const float wiDotWh = localWi.dot(wh);
    const float wiAbsDotWh = util::clamp(localWi.absDot(wh), 0.f, 1.);
    Logger::cout << "Rough eval fresnel args: " << wiDotWh << " " << etaIncident << " " << etaTransmitted << std::endl;
    const float fresnel = Fresnel::dielectricReflectance(
        wiAbsDotWh,
        etaIncident,
        etaTransmitted
    );

    Logger::cout << "Rough eval F: " << fresnel << std::endl;

    const float woDotWh = localWo.dot(wh);
    const float woAbsDotWh = localWo.absDot(wh);

    if (cosThetaI == 0.f || cosThetaO == 0.f) { return Color(0.f); }
    if (wh.isZero()) { return Color(0.f); }

    const float distribution = m_distributionPtr->D(wh);
    const float masking = m_distributionPtr->G(localWi, localWo);

    const Color albedo(1.f);

    if (isReflect) {
        *pdf = m_distributionPtr->pdf(wh) * reflectJacobian(woAbsDotWh);

        Logger::cout << "Rough eval reflect PDF: " << *pdf << std::endl;

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

        Logger::cout << "Rough eval refract PDF: " << *pdf << std::endl;

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

        assert(!std::isinf(value.r()));
        assert(!std::isnan(value.r()));

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
    const Vector3 localWi = buildLocalWi(intersection);
    const Vector3 wh = m_distributionPtr->sampleWh(localWi, random);

    float etaIncident = 1.f;
    float etaTransmitted = m_ior;

    if (localWi.y() < 0.f) {
        std::swap(etaIncident, etaTransmitted);
    }

    const float wiDotWh = localWi.dot(wh);
    const float wiAbsDotWh = util::clamp(localWi.absDot(wh), 0.f, 1.f);

    Logger::cout << "Rough sample w_h: " << wh.toString() << std::endl;
    Logger::cout << "Rough sample fresnel args: " << wiDotWh << " " << etaIncident << " " << etaTransmitted << std::endl;
    const float fresnelReflectance = Fresnel::dielectricReflectance(
        wiAbsDotWh,
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

        Logger::cout << "Calculating throughput..." << std::endl;
        const Color throughput = Material::f(intersection, woWorld);
        Logger::cout << "Done! (" << throughput << ")" << std::endl;

        const float pdf = m_distributionPtr->pdf(wh)
            * jacobian
            * fresnelTransmittance;

        Logger::cout << "Calculating PDF..." << std::endl
                     << "  PDF: " << pdf << std::endl
                     << "  D term: " << m_distributionPtr->pdf(wh) << std::endl
                     << "  Jacobian: " << jacobian << std::endl
                     << "  Fresnel: " << fresnelTransmittance << std::endl
                     << "Done!" << std::endl;

        const BSDFSample sample = {
            .wiWorld = woWorld,
            .pdf = pdf,
            .throughput = throughput,
            .material = this
        };

        return sample;
    }
}
