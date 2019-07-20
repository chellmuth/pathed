#pragma once

#include "point.h"
#include "ray.h"
#include "vector.h"

namespace geometry {
    struct Plane {
        Vector3 normal;
        Point3 point;

        Plane(const Vector3 &normal_, const Point3 &point_)
        : normal(normal_), point(point_) {}
    };

    Point3 intersectRayPlane(const Ray &ray, const Plane &plane);
};
