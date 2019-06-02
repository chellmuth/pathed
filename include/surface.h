#pragma once

#include <memory>
#include <vector>

#include "intersection.h"
#include "material.h"
#include "primitive.h"
#include "random_generator.h"
#include "shape.h"

class Ray;

class Surface : public Primitive {
public:
    Surface(std::shared_ptr<Shape> shape, std::shared_ptr<Material> material);

    SurfaceSample sample(RandomGenerator &random) const;
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
