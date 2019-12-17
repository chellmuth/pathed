#pragma once

#include "vector.h"

namespace Snell {
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
