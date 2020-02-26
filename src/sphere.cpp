#include "sphere.h"

#include "coordinate.h"
#include "globals.h"
#include "measure.h"
#include "ray.h"
#include "trig.h"
#include "transform.h"
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

static float UniformConePdf(float cosThetaMax)
{
    return 1.f / (2.f * M_PI * (1.f - cosThetaMax));
}

SurfaceSample Sphere::sample(
    const Point3 &referencePoint,
    RandomGenerator &random
) const {
    // early quit if we're inside the sphere
    const float centerDistance = (m_center - referencePoint).toVector().length();
    const float centerDistance2 = centerDistance * centerDistance;
    if (centerDistance <= m_radius) {
        return sample(random);
    }

    // compute angle of cone bounding what referencePoint can "see" on sphere
    const float radius2 = m_radius * m_radius;
    const float sin2ThetaMax = m_radius * m_radius / centerDistance2;
    const float cosThetaMax = Trig::cosFromSin2(sin2ThetaMax);

    // linearly interpolate between [cosThetaMax, 1];
    // theta will be between [0, cosThetaMax]
    const float xi1 = random.next();
    const float cosTheta = (1.f - xi1) + xi1 * cosThetaMax;
    const float phi = random.next() * 2.f * M_PI;

    // compute distance to sample point on sphere
    const float sinTheta = Trig::sinFromCos(cosTheta);
    const float sideOppositeTheta = centerDistance * sinTheta;
    const float sideHelper = std::sqrt(
        std::max(0.f, m_radius * m_radius - sideOppositeTheta * sideOppositeTheta)
    );
    const float sampleDistance = centerDistance * cosTheta - sideHelper;
    const float sampleDistance2 = sampleDistance * sampleDistance;

    // compute internal angle to sample
    const float cosAlpha = util::clampClose(
        (sampleDistance2 - centerDistance2 - radius2)
        / (2.f * m_radius * centerDistance),
        0.f, 1.f
    );
    const float sinAlpha = Trig::sinFromCos(cosAlpha);

    const Vector3 localSample = sphericalToCartesian(phi, cosAlpha, sinAlpha);
    const Transform localToWorld = normalToWorldSpace((referencePoint - m_center).toVector().normalized());
    const Vector3 worldSample = localToWorld.apply(localSample);

    SurfaceSample sample = {
        .point = m_center + worldSample * m_radius,
        .normal = worldSample.normalized(),
        .invPDF = 1.f / UniformConePdf(cosThetaMax),
        .measure = Measure::SolidAngle
    };

    return sample;
}

float Sphere::pdf(const Point3 &point, const Point3 &referencePoint) const
{
    // Redo some of the sampling logic to generate cosThetaMax
    const float centerDistance = (m_center - referencePoint).toVector().length();
    const float centerDistance2 = centerDistance * centerDistance;
    if (centerDistance <= m_radius) {
        std::cout << "TODO: Needs to be converted to solid angle measure" << std::endl;
        return pdf(point);
    }

    const float radius2 = m_radius * m_radius;
    const float sin2ThetaMax = m_radius * m_radius / centerDistance2;
    const float cosThetaMax = Trig::cosFromSin2(sin2ThetaMax);

    return UniformConePdf(cosThetaMax);
}

float Sphere::pdf(const Point3 &point) const
{
    return 1.f / area();
}

float Sphere::area() const
{
    return 4 * M_PI * m_radius * m_radius;
}
