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
