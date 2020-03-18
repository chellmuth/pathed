#include "quad.h"

#include "globals.h"
#include "point.h"
#include "triangle.h"

static const Point3 yUpPoints[] = {
    Point3(-1.f, 0.f, -1.f),
    Point3(-1.f, 0.f, 1.f),
    Point3(1.f, 0.f, -1.f),

    Point3(-1.f, 0.f, 1.f),
    Point3(1.f, 0.f, 1.f),
    Point3(1.f, 0.f, -1.f),
};

static const Point3 zUpPoints[] = {
    Point3(-1.f, -1.f, 0.f),
    Point3(1.f, -1.f, 0.f),
    Point3(-1.f, 1.f, 0.f),

    Point3(-1.f, 1.f, 0.f),
    Point3(1.f, -1.f, 0.f),
    Point3(1.f, 1.f, 0.f),
};

static const UV zUpUVs[] = {
    { 0.f, 0.f, },
    { 1.f, 0.f, },
    { 0.f, 1.f, },
    { 0.f, 1.f, },
    { 1.f, 0.f, },
    { 1.f, 1.f, },
};

static const UV yUpUVs[] = {
    { 0.f, 0.f, },
    { 0.f, 1.f, },
    { 1.f, 0.f, },
    { 0.f, 1.f, },
    { 1.f, 1.f, },
    { 1.f, 0.f, },
};

void Quad::parse(
    const Transform &transform,
    std::shared_ptr<Material> material,
    std::shared_ptr<Medium> internalMedium,
    std::vector<std::shared_ptr<Surface>> &surfaces,
    Axis upAxis
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
        Point3(0.f, 0.f, 0.f),
        Point3(0.f, 0.f, 0.f),
        Point3(0.f, 0.f, 0.f),
        Point3(0.f, 0.f, 0.f),
        Point3(0.f, 0.f, 0.f),
        Point3(0.f, 0.f, 0.f),
    };

    for (int i = 0; i < 6; i++) {
        if (upAxis == Axis::Y) {
            points[i] = yUpPoints[i];
        } else if (upAxis == Axis::Z) {
            points[i] = zUpPoints[i];
        }

        points[i] = transform.apply(points[i]);
    }

    UV uvs[] = {
        { 0.f, 0.f },
        { 0.f, 0.f },
        { 0.f, 0.f },
        { 0.f, 0.f },
        { 0.f, 0.f },
        { 0.f, 0.f },
    };

    for (int i = 0; i < 6; i++) {
        if (upAxis == Axis::Y) {
            uvs[i] = yUpUVs[i];
        } else if (upAxis == Axis::Z) {
            uvs[i] = zUpUVs[i];
        }
    }

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

    auto surface1 = std::make_shared<Surface>(triangle1, material, internalMedium);
    surfaces.push_back(surface1);

    auto surface2 = std::make_shared<Surface>(triangle2, material, internalMedium);
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

    Vector3 normal = upAxis == Axis::Y
        ? Vector3(0.f, 1.f, 0.f)
        : Vector3(0.f, 0.f, 1.f)
    ;

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
