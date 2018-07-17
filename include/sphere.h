#pragma once

#include "color.h"
#include "intersection.h"
#include "point.h"

class Ray;

class Sphere {
public:
    Sphere(Point3 center, float radius, Color color);

    Intersection testIntersect(const Ray &ray);

private:
    Point3 m_center;
    float m_radius;
    Color m_color;
};
