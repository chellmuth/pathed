#pragma once

#include "intersection.h"
#include "material.h"
#include "point.h"
#include "shape.h"
#include "transform.h"

#include <memory>

class Ray;

class Curve : public Shape {
public:
    Curve(Point3 p1, Point3 p2, Point3 p3, Point3 p4, float width0, float width1);

    SurfaceSample sample(RandomGenerator &random) const override { throw "Unimplemented!"; }
    float pdf(const Point3 &point) const override { throw "Unimplemented!"; }

    float area() const override { throw "Unimplemented!"; }

    void create(
        const Transform &transform,
        std::shared_ptr<Material> material
    );

    bool useBackwardsNormals() const override { return false; }

    Point3 p1() const { return m_p1; }
    Point3 p2() const { return m_p2; }
    Point3 p3() const { return m_p3; }
    Point3 p4() const { return m_p4; }
    float width0() const { return m_width0; }

private:
    Point3 m_p1, m_p2, m_p3, m_p4;
    float m_width0, m_width1;
};
