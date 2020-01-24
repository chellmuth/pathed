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

    Point3 point() const {
        return isSurface
            ? intersection.point
            : scatterEvent.point
        ;
    }

    Vector3 wiWorld() const {
        return isSurface
            ? bsdfSample.wiWorld
            : scatterEvent.wiWorld
        ;
    }
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
