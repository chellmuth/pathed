#include "snell.h"

#include "tangent_frame.h"
#include "trig.h"

#include <algorithm>
#include <cmath>

bool Snell::refract(
    const Vector3 &incidentLocal,
    Vector3 *transmittedLocal,
    float etaIncident,
    float etaTransmitted
) {
    Vector3 normal = Vector3(0.f, 1.f, 0.f);
    if (incidentLocal.y() < 0.f) {
        normal = normal * -1.f;
    }
    const Vector3 wIncidentPerpendicular = incidentLocal - (normal * incidentLocal.dot(normal));
    const Vector3 wTransmittedPerpendicular = -wIncidentPerpendicular * (etaIncident / etaTransmitted);

    const float transmittedPerpendicularLength2 = wTransmittedPerpendicular.length() * wTransmittedPerpendicular.length();
    const float wTransmittedParallelLength = sqrtf(std::max(0.f, 1.f - transmittedPerpendicularLength2));
    const Vector3 wTransmittedParallel = normal * -wTransmittedParallelLength;

    const float cosThetaIncident = TangentFrame::cosTheta(incidentLocal);
    const float sin2ThetaIncident = Trig::sin2FromCos(cosThetaIncident);
    const float eta2 = (etaIncident / etaTransmitted) * (etaIncident / etaTransmitted);
    const float sin2ThetaTransmitted = eta2 * sin2ThetaIncident;

    *transmittedLocal = (wTransmittedParallel + wTransmittedPerpendicular).normalized();

    if (sin2ThetaTransmitted >= 1.f) { // total internal reflection
        return false;
    }
    return true;
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
    return (etaIncident / etaTransmitted) * Trig::sinThetaFromCosTheta(cosThetaIncident);
}
