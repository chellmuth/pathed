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

    std::shared_ptr<Shape> getShape() const;
    std::shared_ptr<Material> getMaterial() const;
    std::shared_ptr<Medium> getInternalMedium() const { return m_internalMedium; }

    Color getRadiance() const;

private:
    std::shared_ptr<Shape> m_shape;
    std::shared_ptr<Material> m_material;
    std::shared_ptr<Medium> m_internalMedium;
};
