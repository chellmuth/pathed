#include "triangle.h"

#include "measure.h"
#include "point.h"
#include "ray.h"
#include "vector.h"

#include <limits>
#include <math.h>
#include <stdio.h>

Triangle::Triangle(
    Point3 p0, Point3 p1, Point3 p2,
    UV uv0, UV uv1, UV uv2
)
    : m_p0(p0), m_p1(p1), m_p2(p2),
      m_hasUVs(true),
      m_uv0(uv0), m_uv1(uv1), m_uv2(uv2)
{}

Triangle::Triangle(Point3 p0, Point3 p1, Point3 p2)
    : m_p0(p0), m_p1(p1), m_p2(p2),
      m_hasUVs(false)
{}

SurfaceSample Triangle::sample(RandomGenerator &random) const
{
    float r1 = random.next();
    float r2 = random.next();

    float a = 1 - sqrt(r1);
    float b = sqrt(r1) * (1 - r2);
    float c = 1 - a - b;

    Point3 point = m_p0 * a + m_p1 * b + m_p2 * c;

    Vector3 e1 = (m_p1 - m_p0).toVector();
    Vector3 e2 = (m_p2 - m_p0).toVector();
    Vector3 normal = e2.cross(e1).normalized();

    SurfaceSample sample = {
        .point = point,
        .normal = normal,
        .invPDF = area(),
        .measure = Measure::Area
    };
    return sample;
}

float Triangle::pdf(const Point3 &point) const
{
    return 1.f / area();
}

Intersection Triangle::testIntersect(const Ray &ray)
{
    Intersection miss = IntersectionHelper::miss;

    Vector3 e1 = (m_p1 - m_p0).toVector();
    Vector3 e2 = (m_p2 - m_p0).toVector();

    Vector3 s1 = ray.direction().cross(e2);
    float divisor = s1.dot(e1);

    if (divisor == 0.) { return miss; }

    float inverseDivisor = 1.f / divisor;

    Vector3 s = (ray.origin() - m_p0).toVector();
    float b1 = s.dot(s1) * inverseDivisor;

    if (b1 < 0.f || b1 > 1.f) { return miss; }

    Vector3 s2 = s.cross(e1);
    float b2 = ray.direction().dot(s2) * inverseDivisor;

    if (b2 < 0.f || (b1 + b2) > 1.f) { return miss; }

    float t = e2.dot(s2) * inverseDivisor;
    if (t <= 0.001f) { return miss; }

    Point3 hitPoint = m_p0 * (1.f - b1 - b2)  + (m_p1 * b1) + (m_p2 * b2);

    UV uv = { 0.f, 0.f };
    if (m_hasUVs) {
        uv = {
            .u = m_uv0.u * (1.f - b1 - b2) + (m_uv1.u * b1) + (m_uv2.u * b2),
            .v = m_uv0.v * (1.f - b1 - b2) + (m_uv1.v * b1) + (m_uv2.v * b2),
        };
    }

    Intersection hit = {
        .hit = true,
        .t = t,
        .point = hitPoint,
        .woWorld = -ray.direction(),
        .normal = e2.cross(e1).normalized(),
        .shadingNormal = e2.cross(e1).normalized(),
        .uv = uv,
        .material = nullptr,
        .surface = nullptr
    };

    return hit;
}

float Triangle::area() const
{
    Vector3 e1 = (m_p1 - m_p0).toVector();
    Vector3 e2 = (m_p2 - m_p0).toVector();

    Vector3 cross = e1.cross(e2);
    return fabsf(cross.length() / 2.f);
}

void Triangle::pushVertices(std::vector<float> &vertices)
{
    vertices.push_back(m_p0.x());
    vertices.push_back(m_p0.y());
    vertices.push_back(m_p0.z());

    vertices.push_back(m_p1.x());
    vertices.push_back(m_p1.y());
    vertices.push_back(m_p1.z());

    vertices.push_back(m_p2.x());
    vertices.push_back(m_p2.y());
    vertices.push_back(m_p2.z());
}

void Triangle::pushIndices(std::vector<uint> &indices, int offset)
{
    indices.push_back(offset + 0);
    indices.push_back(offset + 1);
    indices.push_back(offset + 2);
}

void Triangle::pushNormals(std::vector<float> &normals)
{
    Vector3 e1 = (m_p1 - m_p0).toVector();
    Vector3 e2 = (m_p2 - m_p0).toVector();
    Vector3 normal = e2.cross(e1).normalized();

    normals.push_back(normal.x());
    normals.push_back(normal.y());
    normals.push_back(normal.z());

    normals.push_back(normal.x());
    normals.push_back(normal.y());
    normals.push_back(normal.z());

    normals.push_back(normal.x());
    normals.push_back(normal.y());
    normals.push_back(normal.z());
}

void Triangle::debug() const
{
    printf("<Triangle>\n");
    m_p0.debug();
    m_p1.debug();
    m_p2.debug();
    printf("</Triangle>\n");
}
