#pragma once

#include "intersection.h"
#include "material.h"
#include "measure.h"
#include "point.h"
#include "shape.h"
#include "transform.h"

#include <embree3/rtcore.h>

#include <memory>
#include <vector>

class Ray;

class BSpline : public Shape {
public:
    BSpline(std::vector<Point3>, float width0, float width1);

    SurfaceSample sample(RandomGenerator &random) const override { throw "Unimplemented!"; }
    float pdf(const Point3 &point, Measure measure) const override { throw "Unimplemented!"; }

    float area() const override { throw "Unimplemented!"; }

    bool useBackwardsNormals() const override { return false; }

    const std::vector<Point3> &points() const { return m_points; }
    float width0() const { return m_width0; }
    float width1() const { return m_width1; }

private:
    std::vector<Point3> m_points;
    float m_width0, m_width1;
};
