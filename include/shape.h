#pragma once

#include "intersection.h"
#include "point.h"
#include "random_generator.h"
#include "vector.h"

#include <vector>

class Ray;

typedef struct {
    Point3 point;
    Vector3 normal;
    float invPDF;
} SurfaceSample;

class Shape {
public:
    virtual void pushVertices(std::vector<float> &vertices) {};
    virtual void pushNormals(std::vector<float> &normals) {};
    virtual void pushIndices(std::vector<uint> &indices, int offset) {};

    virtual SurfaceSample sample(RandomGenerator &random) const = 0;
    virtual Intersection testIntersect(const Ray &ray) = 0;
};
