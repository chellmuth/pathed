#include "curve.h"

#include "globals.h"

#include <embree3/rtcore.h>

Curve::Curve(Point3 p1, Point3 p2, Point3 p3, Point3 p4, float width0, float width1)
    : m_p1(p1),
      m_p2(p2),
      m_p3(p3),
      m_p4(p4),
      m_width0(width0)
{}

void Curve::create(
    const Transform &transform,
    std::shared_ptr<Material> material
) {
    RTCGeometry rtcMesh = rtcNewGeometry(g_rtcDevice, RTC_GEOMETRY_TYPE_FLAT_BEZIER_CURVE);

    float *rtcVertices = (float *)rtcSetNewGeometryBuffer(
        rtcMesh,                /* geometry */
        RTC_BUFFER_TYPE_VERTEX, /* type */
        0,                      /* slot */
        RTC_FORMAT_FLOAT4,      /* format */
        4 * sizeof(float),      /* byte stride */
        4                       /* item count */
    );

    unsigned int *rtcIndices = (unsigned int *)rtcSetNewGeometryBuffer(
        rtcMesh,
        RTC_BUFFER_TYPE_INDEX,
        0,
        RTC_FORMAT_UINT,
        1 * sizeof(unsigned int),
        1
    );

    rtcVertices[0 + 0] = m_p1.x();
    rtcVertices[0 + 1] = m_p1.y();
    rtcVertices[0 + 2] = m_p1.z();
    rtcVertices[0 + 3] = m_width0;

    rtcVertices[4 + 0] = m_p2.x();
    rtcVertices[4 + 1] = m_p2.y();
    rtcVertices[4 + 2] = m_p2.z();
    rtcVertices[4 + 3] = m_width0;

    rtcVertices[8 + 0] = m_p3.x();
    rtcVertices[8 + 1] = m_p3.y();
    rtcVertices[8 + 2] = m_p3.z();
    rtcVertices[8 + 3] = m_width0;

    rtcVertices[12 + 0] = m_p4.x();
    rtcVertices[12 + 1] = m_p4.y();
    rtcVertices[12 + 2] = m_p4.z();
    rtcVertices[12 + 3] = m_width0;

    rtcIndices[0] = 0;

    rtcCommitGeometry(rtcMesh);

    unsigned int rtcGeometryID = rtcAttachGeometry(g_rtcScene, rtcMesh);
    rtcReleaseGeometry(rtcMesh);
}
