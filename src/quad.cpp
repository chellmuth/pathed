#include "quad.h"

#include "globals.h"
#include "point.h"
#include "triangle.h"

void Quad::parse(
    const Transform &transform,
    std::shared_ptr<Material> material,
    std::vector<std::shared_ptr<Surface>> &surfaces
) {
    RTCGeometry rtcMesh = rtcNewGeometry(g_rtcDevice, RTC_GEOMETRY_TYPE_TRIANGLE);
    float *rtcVertices = (float *)rtcSetNewGeometryBuffer(
        rtcMesh,                /* geometry */
        RTC_BUFFER_TYPE_VERTEX, /* type */
        0,                      /* slot */
        RTC_FORMAT_FLOAT3,      /* format */
        3 * sizeof(float),      /* byte stride */
        6                       /* item count */
    );

    Point3 points[] = {
        transform.apply(Point3(-1.f, -1.f, 0.f)),
        transform.apply(Point3(-1.f, 1.f, 0.f)),
        transform.apply(Point3(1.f, -1.f, 0.f)),

        transform.apply(Point3(-1.f, 1.f, 0.f)),
        transform.apply(Point3(1.f, 1.f, 0.f)),
        transform.apply(Point3(1.f, -1.f, 0.f))
    };

    UV uvs[] = {
        { 0.f, 0.f, },
        { 0.f, 1.f, },
        { 1.f, 0.f, },
        { 0.f, 1.f, },
        { 1.f, 1.f, },
        { 1.f, 0.f, },
    };

    std::shared_ptr<Shape> triangle1 = std::make_shared<Triangle>(
        points[0],
        points[1],
        points[2]
    );
    std::shared_ptr<Shape> triangle2 = std::make_shared<Triangle>(
        points[3],
        points[4],
        points[5]
    );

    auto surface1 = std::make_shared<Surface>(triangle1, material);
    surfaces.push_back(surface1);

    auto surface2 = std::make_shared<Surface>(triangle2, material);
    surfaces.push_back(surface2);

    for (int i = 0; i < 6; i++) {
        rtcVertices[i * 3 + 0] = points[i].x();
        rtcVertices[i * 3 + 1] = points[i].y();
        rtcVertices[i * 3 + 2] = points[i].z();
    }

    unsigned int *rtcFaces = (unsigned int *)rtcSetNewGeometryBuffer(
        rtcMesh,
        RTC_BUFFER_TYPE_INDEX,
        0,
        RTC_FORMAT_UINT3,
        3 * sizeof(unsigned int),
        2
    );

    for (int i = 0; i < 6; i++) {
        rtcFaces[i] = i;
    }

    rtcSetGeometryVertexAttributeCount(rtcMesh, 2);

    float *rtcUVs = (float *)rtcSetNewGeometryBuffer(
        rtcMesh,                          /* geometry */
        RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, /* type */
        0,                                /* slot */
        RTC_FORMAT_FLOAT2,                /* format */
        2 * sizeof(float),                /* byte stride */
        3 * 2                             /* item count */
    );
    for (int i = 0; i < 3 * 2; i++) {
        UV uv = uvs[i];

        rtcUVs[2 * i + 0] = uv.u;
        rtcUVs[2 * i + 1] = uv.v;
    }

    float *rtcNormals = (float *)rtcSetNewGeometryBuffer(
        rtcMesh,                          /* geometry */
        RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, /* type */
        1,                                /* slot */
        RTC_FORMAT_FLOAT3,                /* format */
        3 * sizeof(float),                /* byte stride */
        3 * 2                             /* item count */
    );

    Vector3 normal = Vector3(0.f, 0.f, 1.f);
    for (int i = 0; i < 3 * 2; i++) {
        Vector3 transformedNormal = transform.apply(normal).normalized();

        rtcNormals[3 * i + 0] = transformedNormal.x();
        rtcNormals[3 * i + 1] = transformedNormal.y();
        rtcNormals[3 * i + 2] = transformedNormal.z();
    }

    rtcCommitGeometry(rtcMesh);

    unsigned int rtcGeometryID = rtcAttachGeometry(g_rtcScene, rtcMesh);
    rtcReleaseGeometry(rtcMesh);
}
