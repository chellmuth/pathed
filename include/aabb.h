#pragma once

#include "point.h"
#include "ray.h"

struct AABBHit {
    bool isHit;

    Point3 enterPoint;
    Point3 exitPoint;

    float enterT;
    float exitT;
};

class AABB {
public:
    AABB(float minX, float minY, float minZ, float maxX, float maxY, float maxZ);

    AABBHit intersect(const Ray &ray);
    AABBHit intersect(const Point3 &enterPoint, const Point3 &exitPoint);

private:
    float m_maxX, m_minX;
    float m_maxY, m_minY;
    float m_maxZ, m_minZ;
};
