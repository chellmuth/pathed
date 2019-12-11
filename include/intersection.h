#pragma once

#include "material.h"
#include "point.h"
#include "transform.h"
#include "uv.h"
#include "vector.h"

#include <limits>

struct Intersection {
    bool hit;
    float t;
    Point3 point;
    Vector3 woWorld;
    Vector3 normal;
    Vector3 shadingNormal;
    UV uv;
    Material *material;

    Transform tangentToWorld;
    Transform worldToTangent;

    Intersection(
        bool hit_,
        float t_,
        Point3 point_,
        Vector3 woWorld_,
        Vector3 normal_,
        Vector3 shadingNormal_,
        UV uv_,
        Material *material
    ) : hit(hit_),
        t(t_),
        point(point_),
        woWorld(woWorld_),
        normal(normal_),
        shadingNormal(shadingNormal_),
        uv(uv_),
        material(material)
    {
        tangentToWorld = normalToWorldSpace(shadingNormal, woWorld);
        worldToTangent = worldSpaceToNormal(shadingNormal, woWorld);
    }

    bool isEmitter() {
        return !(material->emit().isBlack());
    }
};

namespace IntersectionHelper {
    const Intersection miss(
        false,
        std::numeric_limits<float>::max(),
        Point3(0.f, 0.f, 0.f),
        Vector3(0.f),
        Vector3(0.f),
        Vector3(0.f),
        { 0.f, 0.f },
        nullptr
    );

    inline bool checkBacksideIntersection(const Intersection &intersection)
    {
        return intersection.normal.dot(intersection.woWorld) < 0.f;
    }
}
