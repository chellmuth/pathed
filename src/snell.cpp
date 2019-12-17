#include "snell.h"

#include "tangent_frame.h"
#include "trig.h"

#include <algorithm>
#include <cmath>

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
    return (etaIncident / etaTransmitted) * Trig::sinThetaFromCosTheta(cosThetaIncident);
}
