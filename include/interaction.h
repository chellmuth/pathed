#pragma once

#include "material.h"
#include "intersection.h"
#include "medium.h"
#include "point.h"
#include "vector.h"

struct ScatterEvent {
    Point3 point;
    Vector3 woWorld;
    Vector3 wiWorld;
};

struct Interaction {
    bool isSurface;

    // Surface event
    Intersection intersection;
    BSDFSample bsdfSample;

    // Volume event
    ScatterEvent scatterEvent;
};

namespace InteractionHelper {
    inline ScatterEvent nullScatter() {
        return ScatterEvent({
            Point3(0.f, 0.f, 0.f),
            Vector3(0.f),
            Vector3(0.f)
        });
    }
};
