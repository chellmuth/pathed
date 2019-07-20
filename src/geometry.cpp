#include "geometry.h"

Point3 geometry::intersectRayPlane(const Ray &ray, const Plane &plane)
{
    const float numerator = plane.normal.dot(plane.point - ray.origin());
    const float denominator = plane.normal.dot(ray.direction());
    const float t = numerator / denominator;

    return ray.at(t);
}
