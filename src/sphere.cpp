#include "sphere.h"

#include "globals.h"
#include "measure.h"
#include "ray.h"
#include "util.h"

#include <embree3/rtcore.h>

#include <cmath>
#include <limits>

void Sphere::create(
    const Transform &transform,
    std::shared_ptr<Material> material
) {
    RTCGeometry rtcMesh = rtcNewGeometry(g_rtcDevice, RTC_GEOMETRY_TYPE_SPHERE_POINT);
    float *rtcVertices = (float *)rtcSetNewGeometryBuffer(
        rtcMesh,                /* geometry */
        RTC_BUFFER_TYPE_VERTEX, /* type */
        0,                      /* slot */
        RTC_FORMAT_FLOAT4,      /* format */
        4 * sizeof(float),      /* byte stride */
        1                       /* item count */
    );

    float data[] = {
        transform.apply(m_center).x(),
        transform.apply(m_center).y(),
        transform.apply(m_center).z(),
        m_radius
    };

    for (int i = 0; i < 1; i++) {
        rtcVertices[i * 4 + 0] = data[i * 4  + 0];
        rtcVertices[i * 4 + 1] = data[i * 4  + 1];
        rtcVertices[i * 4 + 2] = data[i * 4  + 2];
        rtcVertices[i * 4 + 3] = data[i * 4  + 3];
    }

    rtcCommitGeometry(rtcMesh);

    unsigned int rtcGeometryID = rtcAttachGeometry(g_rtcScene, rtcMesh);
    rtcReleaseGeometry(rtcMesh);
}

Sphere::Sphere(Point3 center, float radius)
    : m_center(center), m_radius(radius)
{}

SurfaceSample Sphere::sample(RandomGenerator &random) const
{
    // from pbrt
    float z = 1 - 2 * random.next();
    float r = sqrt(fmaxf(0, 1 - z * z));
    float phi = 2 * M_PI * random.next();
    Vector3 v(r * cosf(phi), r * sinf(phi), z);

    SurfaceSample sample = {
        .point = m_center + v * m_radius,
        .normal = v.normalized(),
        .invPDF = area(),
        .measure = Measure::Area
    };

    return sample;
}

float Sphere::pdf(const Point3 &point) const
{
    return 1.f / area();
}

Intersection Sphere::testIntersect(const Ray &ray)
{
    Point3 origin = ray.origin();
    Vector3 direction = ray.direction();

    Point3 L = origin - m_center;

    float a = direction.dot(direction);
    float b = 2 * direction.dot(L);
    float c = L.dot(L) - m_radius * m_radius;

    QuadraticSolution solution = solveQuadratic(a, b, c);
    if (solution.hasRealSolutions && solution.solution1 > 0) {
        Point3 hitPoint = ray.at(solution.solution1);
        Intersection result = {
            .hit = true,
            .t = solution.solution1,
            .point = hitPoint,
            .woWorld = -ray.direction(),
            .normal = (hitPoint - m_center).toVector().normalized(),
            .shadingNormal = (hitPoint - m_center).toVector().normalized(),
            .uv = { 0.f, 0.f },
            .material = nullptr,
            .surface = nullptr
        };
        return result;
    } else {
        return IntersectionHelper::miss;
    }
}

float Sphere::area() const
{
    return 4 * M_PI * m_radius * m_radius;
}
