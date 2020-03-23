#include "fresnel.h"

#include "snell.h"
#include "tangent_frame.h"

#include <assert.h>
#include <cmath>
#include <iostream>

static float divide(const float a, const float b)
{
    assert(b != 0.f);

    return a / b;
}

float Fresnel::dielectricReflectanceWalter(
    const Vector3 &wi,
    const Vector3 &wh,
    float etaI,
    float etaT
) {
    const float c = wi.absDot(wh);
    const float invEta2 = (util::square(etaT) / util::square(etaI));
    const float sqrtG = invEta2 - 1 + util::square(c);
    if (sqrtG < 0.f) {
        // TIR
        return 1.f;
    }
    const float g = std::sqrt(sqrtG);

    const float term1 = util::square(g - c) / util::square(g + c);
    const float term2 = 1 + util::square(c * (g + c) - 1) / util::square(c * (g - c) + 1);

    const float F = 0.5f * term1 * term2;
    return F;
}

float Fresnel::dielectricReflectance(
    const Vector3 &incidentDirection,
    float etaIncident,
    float etaTransmitted
) {
    const float cosThetaIncident = TangentFrame::cosTheta(incidentDirection);
    return dielectricReflectance(
        cosThetaIncident,
        etaIncident,
        etaTransmitted
    );
}

float Fresnel::dielectricReflectance(
    float cosThetaIncident,
    float etaIncident,
    float etaTransmitted
) {
    const float sinThetaTransmitted = Snell::transmittedSinTheta(
        cosThetaIncident,
        etaIncident,
        etaTransmitted
    );

    if (sinThetaTransmitted > 1.f) {
        return 1.f; // Total internal reflection
    }

    const float cosThetaTransmitted = sqrtf(std::max(0.f, 1.f - sinThetaTransmitted * sinThetaTransmitted));

    assert(cosThetaIncident >= 0.f);
    assert(cosThetaIncident <= 1.f);

    assert(cosThetaTransmitted >= 0.f);
    assert(cosThetaTransmitted <= 1.f);

    const float rParallel = divide(
        etaTransmitted * cosThetaIncident - etaIncident * cosThetaTransmitted,
        etaTransmitted * cosThetaIncident + etaIncident * cosThetaTransmitted
    );

    const float rPerpendicular = divide(
        etaIncident * cosThetaIncident - etaTransmitted * cosThetaTransmitted,
        etaIncident * cosThetaIncident + etaTransmitted * cosThetaTransmitted
    );

    return 0.5f * (rParallel * rParallel + rPerpendicular * rPerpendicular);
}
