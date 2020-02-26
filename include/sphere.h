#pragma once

#include "intersection.h"
#include "material.h"
#include "point.h"
#include "random_generator.h"
#include "shape.h"
#include "transform.h"

#include <memory>

class Ray;

class Sphere : public Shape {
public:
    Sphere(Point3 center, float radius);

    SurfaceSample sample(RandomGenerator &random) const override;
    SurfaceSample sample(RandomGenerator &random, const Point3 &referencePoint) const;

    float pdf(const Point3 &point) const override;

    float area() const override;

    void create(
        const Transform &transform,
        std::shared_ptr<Material> material
    );

    bool useBackwardsNormals() const override { return false; }

private:
    Point3 m_center;
    float m_radius;
};
