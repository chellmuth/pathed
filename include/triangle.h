#pragma once

#include "intersection.h"
#include "point.h"
#include "shape.h"

class Ray;

class Triangle : public Shape {
public:
    Triangle(Point3 v0, Point3 v1, Point3 v2);

    Point3 v0() const { return m_v0; }
    Point3 v1() const { return m_v1; }
    Point3 v2() const { return m_v2; }

    Intersection testIntersect(const Ray &ray);

private:
    Point3 m_v0, m_v1, m_v2;
};
