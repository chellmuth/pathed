#pragma once

#include "intersection.h"
#include "measure.h"
#include "point.h"
#include "random_generator.h"
#include "transform.h"
#include "vector.h"

#include <memory>
#include <vector>

class Ray;

typedef struct {
    Point3 point;
    Vector3 normal;
    float invPDF;
    Measure measure;
} SurfaceSample;

class Shape {
public:
    virtual void pushVertices(std::vector<float> &vertices) {};
    virtual void pushNormals(std::vector<float> &normals) {};
    virtual void pushIndices(std::vector<uint> &indices, int offset) {};

    virtual SurfaceSample sample(RandomGenerator &random) const = 0;
    virtual float pdf(const Point3 &point) const = 0;

    virtual float area() const = 0;

    virtual void debug() const { printf("Debug not implemented!\n"); };

    virtual bool useBackwardsNormals() const { return true; } // fixme
};
