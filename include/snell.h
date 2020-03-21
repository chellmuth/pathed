#pragma once

#include "vector.h"

namespace Snell {
    bool refract(
        const Vector3 &incidentLocal,
        Vector3 *transmittedLocal,
        Vector3 normal,
        float etaIncident,
        float etaTransmitted
    );

    bool refract(
        const Vector3 &incidentLocal,
        Vector3 *transmittedLocal,
        float etaIncident,
        float etaTransmitted
    );

    Vector3 computeHalfVector(
        const Vector3 incidentLocal,
        const Vector3 outgoingLocal,
        float etaIncident,
        float etaTransmitted,
        bool isReflect
    );

    float transmittedSinTheta(
        const Vector3 &incidentDirection,
        float etaIncident,
        float etaTransmitted
    );

    float transmittedSinTheta(
        float cosThetaIncident,
        float etaIncident,
        float etaTransmitted
    );
}
