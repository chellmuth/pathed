#pragma once

#include <memory>
#include <vector>

#include "material.h"
#include "measure.h"
#include "medium.h"
#include "point.h"
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
        std::shared_ptr<Medium> internalMedium,
        std::shared_ptr<Medium> externalMedium
    );

    Surface(
        std::shared_ptr<Shape> shape,
        std::shared_ptr<Material> material,
        std::shared_ptr<Medium> internalMedium,
        std::shared_ptr<Medium> externalMedium,
        int faceIndex
    );

    SurfaceSample sample(RandomGenerator &random) const;
    SurfaceSample sample(const Point3 &referencePoint, RandomGenerator &random) const;

    float pdf(const Point3 &point, Measure measure) const;
    float pdf(const Point3 &point, const Point3 &referencePoint, Measure measure) const;

    std::shared_ptr<Shape> getShape() const;
    std::shared_ptr<Material> getMaterial() const;
    std::shared_ptr<Medium> getInternalMedium() const { return m_internalMedium; }
    std::shared_ptr<Medium> getExternalMedium() const { return m_externalMedium; }
    int getFaceIndex() const { return m_faceIndex; }

    Color getRadiance() const;

private:
    std::shared_ptr<Shape> m_shape;
    std::shared_ptr<Material> m_material;
    std::shared_ptr<Medium> m_internalMedium;
    std::shared_ptr<Medium> m_externalMedium;
    int m_faceIndex;
};
