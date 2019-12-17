#include "perfect_transmission.h"

#include "snell.h"
#include "tangent_frame.h"
#include "transform.h"

#include <algorithm>
#include <cmath>
#include <iostream>

PerfectTransmission::PerfectTransmission()
    : Material(0.f)
{}

Color PerfectTransmission::f(
    const Intersection &intersection,
    const Vector3 &wiWorld,
    float *pdf
) const
{
    *pdf = 0.f;
    return Color(0.f);
}

BSDFSample PerfectTransmission::sample(
    const Intersection &intersection,
    RandomGenerator &random
) const
{
    Vector3 localWo = intersection.worldToTangent.apply(intersection.woWorld);
    Vector3 localWi(0.f);

    float etaIncident = 1.f;
    float etaTransmitted = 1.4f;

    if (localWo.y() < 0.f) {
        std::swap(etaIncident, etaTransmitted);
    }
    bool doesRefract = Snell::refract(
        localWo,
        &localWi,
        etaIncident,
        etaTransmitted
    );

    if (doesRefract) {
        return {
            .wiWorld = intersection.tangentToWorld.apply(localWi),
            .pdf = 1.f,
            .throughput = Color(1.f / TangentFrame::absCosTheta(localWi)),
            .material = this
        };
    } else {
        return {
            .wiWorld = intersection.tangentToWorld.apply(localWi),
            .pdf = 1.f,
            .throughput = Color(1.f),
            .material = this
        };
    }
}
