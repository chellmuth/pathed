#pragma once

#include "json.hpp"
using json = nlohmann::json;

#include "point.h"
#include "ray.h"

class Sphere {
public:
    Sphere(json sphereJson);
    Sphere(Point3 center, float radius);

    bool testIntersect(const Ray &ray);

private:
    Point3 m_center;
    float m_radius;
};
