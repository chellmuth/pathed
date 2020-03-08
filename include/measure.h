#pragma once

#include "point.h"
#include "vector.h"
#include "world_frame.h"

enum class Measure {
    SolidAngle,
    Area
};

namespace MeasureConversion {
    inline float areaToSolidAngle(
        float areaPDF,
        const Point3 &referencePoint,
        const Point3 &surfacePoint,
        const Vector3 &surfaceNormal
    ) {
        const Vector3 surfaceDirection = (referencePoint - surfacePoint).toVector();
        const Vector3 surfaceWo = surfaceDirection.normalized();
        const float distance = surfaceDirection.length();

        const float distance2 = distance * distance;
        const float projectedArea = WorldFrame::cosTheta(surfaceNormal, surfaceWo);

        const float solidAnglePDF = areaPDF * distance2 / projectedArea;
        return solidAnglePDF;
    }
};
