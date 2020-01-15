#include "aabb.h"

#include <cmath>
#include <limits>

AABB::AABB(float minX, float minY, float minZ, float maxX, float maxY, float maxZ)
    : m_minX(minX),
      m_minY(minY),
      m_minZ(minZ),
      m_maxX(maxX),
      m_maxY(maxY),
      m_maxZ(maxZ)
{}

AABBHit AABB::intersect(const Ray &ray)
{
    Vector3 invDirection(
        1.f / ray.direction().x(),
        1.f / ray.direction().y(),
        1.f / ray.direction().z()
    );

    Point3 origin = ray.origin();

    float t1 = (m_minX - origin.x()) * invDirection.x();
    float t2 = (m_maxX - origin.x()) * invDirection.x();
    float t3 = (m_minY - origin.y()) * invDirection.y();
    float t4 = (m_maxY - origin.y()) * invDirection.y();
    float t5 = (m_minZ - origin.z()) * invDirection.z();
    float t6 = (m_maxZ - origin.z()) * invDirection.z();

    float tmin = fmaxf(fmaxf(fminf(t1, t2), fminf(t3, t4)), fminf(t5, t6));
    float tmax = fminf(fminf(fmaxf(t1, t2), fmaxf(t3, t4)), fmaxf(t5, t6));

    if (tmin >= 0 && !std::isinf(tmin) && tmax >= 0 && !std::isinf(tmax)) {
        return AABBHit({
            2,
            ray.at(tmin),
            ray.at(tmax)
        });
    }

    if (tmax >= 0 && !std::isinf(tmax) && tmin < 0) {
        return AABBHit({
            1,
            ray.at(0.f),
            ray.at(tmax)
        });
    }

    return AABBHit({ 0, Point3(0.f, 0.f, 0.f), Point3(0.f, 0.f, 0.f) });
}
