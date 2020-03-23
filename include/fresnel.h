#pragma once

#include "vector.h"

namespace Fresnel {
    float dielectricReflectanceWalter(
        const Vector3 &wi,
        const Vector3 &wh,
        float etaI,
        float etaT
    );

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
