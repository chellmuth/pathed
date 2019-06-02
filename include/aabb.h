#pragma once

#include "point.h"
#include "ray.h"

#include <vector>

class Triangle;

class AABB {
public:
    AABB();

    bool testHit(const Ray &ray, float maxT);

    void update(const Point3 &point);

    void bake();

    Point3 getCentroid() const { return m_centroid; }

    Point3 bottomLeftFront() const { return Point3(m_minX, m_minY, m_minZ); }
    Point3 topRightBack() const { return Point3(m_maxX, m_maxY, m_maxZ); }

    void debug() const;

private:
    float m_maxX, m_minX;
    float m_maxY, m_minY;
    float m_maxZ, m_minZ;

    Point3 m_centroid;
};
