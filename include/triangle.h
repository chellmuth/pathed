#pragma once

#include "intersection.h"
#include "point.h"
#include "shape.h"

class Ray;

class Triangle : public Shape {
public:
    Triangle(Point3 p0, Point3 p1, Point3 p2);

    Point3 p0() const { return m_p0; }
    Point3 p1() const { return m_p1; }
    Point3 p2() const { return m_p2; }

    Intersection testIntersect(const Ray &ray);

private:
    Point3 m_p0, m_p1, m_p2;
};
