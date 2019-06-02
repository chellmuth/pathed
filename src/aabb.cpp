#include "aabb.h"

#include "color.h"

#include <limits>
#include <math.h>

AABB::AABB()
    : m_minX(std::numeric_limits<float>::max()),
      m_maxX(std::numeric_limits<float>::lowest()),
      m_minY(std::numeric_limits<float>::max()),
      m_maxY(std::numeric_limits<float>::lowest()),
      m_minZ(std::numeric_limits<float>::max()),
      m_maxZ(std::numeric_limits<float>::lowest()),
      m_centroid(Point3(0.f, 0.f, 0.f))
{}

bool AABB::testHit(const Ray &ray, float maxT)
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

    if (tmax < 0) {
        return false;
    }

    if (tmin > tmax) {
        return false;
    }

    return tmin < maxT;
}

void AABB::update(const Point3 &point)
{
    float x = point.x();
    if (x < m_minX) {
        m_minX = x;
    }
    if (x > m_maxX) {
        m_maxX = x;
    }

    float y = point.y();
    if (y < m_minY) {
        m_minY = y;
    }
    if (y > m_maxY) {
        m_maxY = y;
    }

    float z = point.z();
    if (z < m_minZ) {
        m_minZ = z;
    }
    if (z > m_maxZ) {
        m_maxZ = z;
    }
}

void AABB::bake()
{
    m_centroid = Point3(
        (m_minX + m_maxX) * 0.5f,
        (m_minY + m_maxY) * 0.5f,
        (m_minZ + m_maxZ) * 0.5f
    );
}

void AABB::debug() const
{
    printf("<AABB>\n");
    bottomLeftFront().debug();
    topRightBack().debug();
    printf("</AABB>\n");
}
