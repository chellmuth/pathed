#pragma once

#include "vector.h"

namespace Snell {
    float transmittedSinTheta(
        const Vector3 &incidentDirection,
        float etaIncident,
        float etaTransmitted
    );
}

namespace Fresnel {
    float dielectricReflectance(
        const Vector3 &incidentDirection,
        float etaIncident,
        float etaTransmitted
    );
}
