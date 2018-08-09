#pragma once

#include "color.h"
#include "intersection.h"
#include "point.h"
#include "shape.h"

class Ray;

class Sphere : public Shape {
public:
    Sphere(Point3 center, float radius, Color color);

    Point3 sample(RandomGenerator &random) const;
    Intersection testIntersect(const Ray &ray);
private:
    Point3 m_center;
    float m_radius;
    Color m_color;
};
