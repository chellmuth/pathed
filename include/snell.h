#pragma once

#include "vector.h"

namespace Snell {
    bool refract(
        const Vector3 &incidentLocal,
        Vector3 *transmittedLocal,
        float etaIncident,
        float etaTransmitted
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
