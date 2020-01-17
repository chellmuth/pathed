#include "glass.h"

#include "fresnel.h"
#include "snell.h"
#include "tangent_frame.h"
#include "transform.h"

#include <algorithm>
#include <cmath>
#include <iostream>

Glass::Glass(float ior)
    : Material(0.f), m_ior(ior)
{}

Glass::Glass()
    : Glass(1.4f)
{}

Color Glass::f(
    const Intersection &intersection,
    const Vector3 &wiWorld,
    float *pdf
) const
{
    *pdf = 0.f;
    return Color(0.f);
}

BSDFSample Glass::sample(
    const Intersection &intersection,
    RandomGenerator &random
) const
{
    Vector3 localWo = intersection.worldToTangent.apply(intersection.woWorld);
    Vector3 localWi(0.f);

    float etaIncident = 1.f;
    float etaTransmitted = m_ior;

    if (localWo.y() < 0.f) {
        std::swap(etaIncident, etaTransmitted);
    }

    bool doesRefract = Snell::refract(
        localWo,
        &localWi,
        etaIncident,
        etaTransmitted
    );

    const float fresnelReflectance = Fresnel::dielectricReflectance(
        TangentFrame::absCosTheta(localWo),
        etaIncident, etaTransmitted
    );

    if (random.next() < fresnelReflectance) {
        localWi = localWo.reflect(Vector3(0.f, 1.f, 0.f));

        BSDFSample sample = {
            .wiWorld = intersection.tangentToWorld.apply(localWi),
            .pdf = fresnelReflectance,
            .throughput = Color(fresnelReflectance / TangentFrame::absCosTheta(localWi)),
            .material = this
        };

        return sample;
    } else {
        if (!doesRefract) {
            assert(false);
            exit(1);
        }

        const float fresnelTransmittance = 1.f - fresnelReflectance;

        BSDFSample sample = {
            .wiWorld = intersection.tangentToWorld.apply(localWi),
            .pdf = fresnelTransmittance,
            .throughput = Color(fresnelTransmittance / TangentFrame::absCosTheta(localWi)),
            .material = this
        };

        return sample;
    }
}
