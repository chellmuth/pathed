#pragma once

#include "intersection.h"
#include "point.h"
#include "shape.h"
#include "uv.h"

#include <memory>

class Ray;

class Triangle : public Shape {
public:
    Triangle(Point3 p0, Point3 p1, Point3 p2);
    Triangle(
        Point3 p0, Point3 p1, Point3 p2,
        UV uv0, UV uv1, UV uv2
    );

    Point3 p0() const { return m_p0; }
    Point3 p1() const { return m_p1; }
    Point3 p2() const { return m_p2; }

    SurfaceSample sample(RandomGenerator &random) const override;
    float pdf(const Point3 &point) const override;
    Intersection testIntersect(const Ray &ray) override;

    void pushVertices(std::vector<float> &vertices) override;
    void pushIndices(std::vector<uint> &indices, int offset) override;
    void pushNormals(std::vector<float> &normals) override;

    std::shared_ptr<Shape> transform(const Transform &transform) const override;

    void debug() const override;

    float area() const override;

private:
    Point3 m_p0, m_p1, m_p2;

    bool m_hasUVs;
    UV m_uv0, m_uv1, m_uv2;
};
