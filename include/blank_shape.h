#pragma once

#include "point.h"
#include "random_generator.h"
#include "shape.h"

class BlankTriangle : public Shape {
    SurfaceSample sample(RandomGenerator &random) const override { throw "Unimplemented!"; }
    float pdf(const Point3 &point) const override { throw "Unimplemented!"; }

    float area() const override { throw "Unimplemented!"; }

    bool useBackwardsNormals() const override { return true; }
};

class BlankSpline : public Shape {
    SurfaceSample sample(RandomGenerator &random) const override { throw "Unimplemented!"; }
    float pdf(const Point3 &point) const override { throw "Unimplemented!"; }

    float area() const override { throw "Unimplemented!"; }

    bool useBackwardsNormals() const override { return false; }
};
