#include "snell.h"

#include "tangent_frame.h"
#include "trig.h"

#include <algorithm>
#include <cmath>

bool Snell::refract(
    const Vector3 &incidentLocal,
    Vector3 *transmittedLocal,
    Vector3 normal,
    float etaIncident,
    float etaTransmitted
) {
    if (incidentLocal.dot(normal) < 0.f) {
        normal = normal * -1.f;
    }

    const Vector3 wIncidentPerpendicular = incidentLocal - (normal * incidentLocal.dot(normal));
    const Vector3 wTransmittedPerpendicular = -wIncidentPerpendicular * (etaIncident / etaTransmitted);

    const float transmittedPerpendicularLength2 = wTransmittedPerpendicular.length() * wTransmittedPerpendicular.length();
    const float wTransmittedParallelLength = sqrtf(std::max(0.f, 1.f - transmittedPerpendicularLength2));
    const Vector3 wTransmittedParallel = normal * -wTransmittedParallelLength;

    const float cosThetaIncident = incidentLocal.absDot(normal);
    const float sin2ThetaIncident = Trig::sin2FromCos(cosThetaIncident);
    const float eta2 = (etaIncident / etaTransmitted) * (etaIncident / etaTransmitted);
    const float sin2ThetaTransmitted = eta2 * sin2ThetaIncident;

    *transmittedLocal = (wTransmittedParallel + wTransmittedPerpendicular).normalized();

    if (sin2ThetaTransmitted >= 1.f) { // total internal reflection
        return false;
    }
    return true;
}

bool Snell::refract(
    const Vector3 &incidentLocal,
    Vector3 *transmittedLocal,
    float etaIncident,
    float etaTransmitted
) {
    return refract(
        incidentLocal,
        transmittedLocal,
        Vector3(0.f, 1.f, 0.f),
        etaIncident,
        etaTransmitted
    );
}

Vector3 Snell::computeHalfVector(
    const Vector3 incidentLocal,
    const Vector3 outgoingLocal,
    float etaIncident,
    float etaTransmitted,
    bool isReflect
) {
    if (isReflect) {
        return (incidentLocal + outgoingLocal).normalized();
    } else {
        return (
            incidentLocal * etaIncident
            + outgoingLocal * etaTransmitted
        ).negate().normalized();
    }
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
