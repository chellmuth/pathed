#pragma once

#include <memory>
#include <vector>

#include "material.h"
#include "medium.h"
#include "primitive.h"
#include "random_generator.h"
#include "shape.h"

class Ray;
struct Intersection;

class Surface : public Primitive {
public:
    Surface(
        std::shared_ptr<Shape> shape,
        std::shared_ptr<Material> material,
        std::shared_ptr<Medium> internalMedium
    );

    SurfaceSample sample(RandomGenerator &random) const;
    float pdf(const Point3 &point) const;

    Intersection testIntersect(const Ray &ray) const override;

    std::shared_ptr<Shape> getShape() const;
    std::shared_ptr<Material> getMaterial() const;
    Color getRadiance() const;

    Point3 centroid() const override { return m_shape->centroid(); }
    void updateAABB(AABB *aabb) override { return m_shape->updateAABB(aabb); }


private:
    std::shared_ptr<Shape> m_shape;
    std::shared_ptr<Material> m_material;
};
