#pragma once

#include "ray.h"

struct AABBHit {
    int hitCount;

    float enterTime;
    float exitTime;
};

class AABB {
public:
    AABB(float minX, float minY, float minZ, float maxX, float maxY, float maxZ);

    AABBHit intersect(const Ray &ray);

private:
    float m_maxX, m_minX;
    float m_maxY, m_minY;
    float m_maxZ, m_minZ;
};
