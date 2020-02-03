#include "b_spline.h"

#include "globals.h"

#include <embree3/rtcore.h>

BSpline::BSpline(std::vector<Point3> points, float width0, float width1)
    : m_points(points),
      m_width0(width0),
      m_width1(width1)
{}

void BSpline::create(
    const Transform &transform,
    std::shared_ptr<Material> material
) {
    RTCGeometry rtcMesh = rtcNewGeometry(g_rtcDevice, RTC_GEOMETRY_TYPE_ROUND_BSPLINE_CURVE);

    float *rtcVertices = (float *)rtcSetNewGeometryBuffer(
        rtcMesh,                /* geometry */
        RTC_BUFFER_TYPE_VERTEX, /* type */
        0,                      /* slot */
        RTC_FORMAT_FLOAT4,      /* format */
        4 * sizeof(float),      /* byte stride */
        m_points.size()         /* item count */
    );

    unsigned int *rtcIndices = (unsigned int *)rtcSetNewGeometryBuffer(
        rtcMesh,
        RTC_BUFFER_TYPE_INDEX,
        0,
        RTC_FORMAT_UINT,
        1 * sizeof(unsigned int),
        m_points.size() - 3
    );

    const size_t count = m_points.size();
    for (int i = 0; i < count; i++) {
        const Point3 &point = m_points[i];
        rtcVertices[i * 4 + 0] = point.x();
        rtcVertices[i * 4 + 1] = point.y();
        rtcVertices[i * 4 + 2] = point.z();
        rtcVertices[i * 4 + 3] = m_width0 * (count - 1 - i) + m_width1 * i;

        rtcIndices[i] = i;
    }

    rtcCommitGeometry(rtcMesh);

    unsigned int rtcGeometryID = rtcAttachGeometry(g_rtcScene, rtcMesh);
    rtcReleaseGeometry(rtcMesh);
}
