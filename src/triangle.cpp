#include "triangle.h"

#include "measure.h"
#include "point.h"
#include "ray.h"
#include "vector.h"

#include <limits>
#include <math.h>
#include <stdio.h>

Triangle::Triangle(Point3 p0, Point3 p1, Point3 p2)
    : m_p0(p0), m_p1(p1), m_p2(p2)
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
    Vector3 normal = e1.cross(e2).normalized();

    SurfaceSample sample = {
        .point = point,
        .normal = normal,
        .invPDF = area(),
        .measure = Measure::Area
    };
    return sample;
}

float Triangle::pdf(const Point3 &point, Measure measure) const
{
    if (measure != Measure::Area) {
        throw std::runtime_error("Unsupported measure");
    }

    return 1.f / area();
}

float Triangle::pdf(const Point3 &point, const Point3 &referencePoint, Measure measure) const
{
    const float areaPDF = pdf(point, Measure::Area);

    if (measure == Measure::Area) {
        return areaPDF;
    }

    const Vector3 e1 = (m_p1 - m_p0).toVector();
    const Vector3 e2 = (m_p2 - m_p0).toVector();
    const Vector3 normal = e1.cross(e2).normalized();

    return MeasureConversion::areaToSolidAngle(areaPDF, referencePoint, point, normal);
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
    Vector3 normal = e1.cross(e2).normalized();

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
