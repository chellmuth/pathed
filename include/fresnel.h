#pragma once

#include "vector.h"

namespace Fresnel {
    float dielectricReflectance(
        const Vector3 &incidentDirection,
        float etaIncident,
        float etaTransmitted
    );

    float dielectricReflectance(
        float cosThetaIncident,
        float etaIncident,
        float etaTransmitted
    );
}
