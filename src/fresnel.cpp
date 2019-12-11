#include "fresnel.h"

#include "tangent_frame.h"

#include <assert.h>
#include <cmath>
#include <iostream>

static float divide(const float a, const float b)
{
    assert(b != 0.f);

    return a / b;
}

static float sinThetaFromCosTheta(float cosTheta)
{
    const float result = sqrtf(std::max(0.f, 1.f - cosTheta * cosTheta));
    return result;
}

float Snell::transmittedSinTheta(
    const Vector3 &incidentDirection,
    float etaIncident,
    float etaTransmitted
) {
    return transmittedSinTheta(
        TangentFrame::sinTheta(incidentDirection),
        etaIncident,
        etaTransmitted
    );
}

float Snell::transmittedSinTheta(
    float cosThetaIncident,
    float etaIncident,
    float etaTransmitted
) {
    return (etaIncident / etaTransmitted) * sinThetaFromCosTheta(cosThetaIncident);
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
    if (std::isnan(cosThetaIncident)) {
        std::cout << "INPUT NAN!" << std::endl;
        assert(false);
        exit(1);
    }

    const float sinThetaTransmitted = Snell::transmittedSinTheta(
        cosThetaIncident,
        etaIncident,
        etaTransmitted
    );

    if (std::isnan(sinThetaTransmitted)) {
        std::cout << "sin(theta) transmitted nan" << std::endl;
        assert(false);
        exit(1);
    }

    if (sinThetaTransmitted > 1.f) {
        return 1.f; // Total internal reflection
    }

    const float cosThetaTransmitted = sqrtf(std::max(0.f, 1.f - sinThetaTransmitted * sinThetaTransmitted));

    assert (cosThetaIncident >= 0.f);
    assert (cosThetaIncident <= 1.f);

    assert (cosThetaTransmitted <= 0.f);
    assert (cosThetaTransmitted >= -1.f);

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
