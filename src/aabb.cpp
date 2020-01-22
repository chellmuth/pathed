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

static AABBHit miss() {
    return AABBHit({ false, Point3(0.f, 0.f, 0.f), Point3(0.f, 0.f, 0.f) });
}

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

    if (tmin >= tmax) { return miss(); }
    if (tmin < 0 && tmax == 0) { return miss(); }

    if (tmin >= 0 && !std::isinf(tmin) && tmax >= 0 && !std::isinf(tmax)) {
        return AABBHit({
            true,
            ray.at(tmin),
            ray.at(tmax),
            tmin,
            tmax
        });
    }

    if (tmax >= 0 && !std::isinf(tmax) && tmin < 0) {
        return AABBHit({
            true,
            ray.origin(),
            ray.at(tmax),
            0.f,
            tmax
        });
    }

    return miss();
}

AABBHit AABB::intersect(const Point3 &enterPoint, const Point3 &exitPoint)
{
    const Vector3 travelDirection = (exitPoint - enterPoint).toVector();
    const Ray testRay(enterPoint, travelDirection.normalized());

    const AABBHit hit = intersect(testRay);

    const float maxT = travelDirection.length();
    if (!hit.isHit) { return hit; }
    if (hit.exitT <= maxT) { return hit; }

    return AABBHit({
        true,
        hit.enterPoint,
        exitPoint,
        hit.enterT,
        maxT
    });
}

std::ostream &operator<<(std::ostream &os, const AABB &aabb)
{
    return os << "[AABB: "
              << "minX=" << aabb.m_minX << " "
              << "maxX=" << aabb.m_maxX << " "
              << "minY=" << aabb.m_minY << " "
              << "maxY=" << aabb.m_maxY << " "
              << "minZ=" << aabb.m_minZ << " "
              << "maxZ=" << aabb.m_maxZ << "]";
}

std::ostream &operator<<(std::ostream &os, const AABBHit &hit)
{
    return os << "[AABBHit: "
              << "isHit=" << hit.isHit << " "
              << "enterPoint=" << hit.enterPoint.toString() << " "
              << "exitPoint=" << hit.exitPoint.toString() << " "
              << "enterT=" << hit.enterT << " "
              << "exitT=" << hit.exitT << "]";
}

