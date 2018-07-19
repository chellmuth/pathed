#pragma once

#include <list>

#include "intersection.h"
#include "point.h"
#include "sphere.h"


class Ray;

class Scene {
public:
    Scene(std::list<Sphere> objects, Point3 light);

    Point3 light() const { return m_light; }
    Intersection testIntersect(const Ray &ray);

private:
    std::list<Sphere> m_objects;
    Point3 m_light;
};
