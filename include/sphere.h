#pragma once

#include "aabb.h"
#include "color.h"
#include "intersection.h"
#include "point.h"
#include "shape.h"
#include "transform.h"

#include <memory>

class Ray;

class Sphere : public Shape {
public:
    Sphere(Point3 center, float radius, Color color);

    SurfaceSample sample(RandomGenerator &random) const;
    Intersection testIntersect(const Ray &ray);

    Point3 centroid() const override { return m_center; }
    void updateAABB(AABB *aabb) override;

    std::shared_ptr<Shape> transform(const Transform &transform) const override;

private:
    float area() const;

    Point3 m_center;
    float m_radius;
    Color m_color;
};
