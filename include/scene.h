#pragma once

#include <list>

#include "intersection.h"
#include "point.h"
#include "shape.h"

class Ray;

class Scene {
public:
    Scene(std::list<Shape *> objects, Point3 light);

    Point3 light() const { return m_light; }
    Intersection testIntersect(const Ray &ray);

private:
    std::list<Shape *> m_objects;
    Point3 m_light;
};
