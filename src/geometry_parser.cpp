#include "geometry_parser.h"

#include "globals.h"

#include <embree3/rtcore.h>

void GeometryParser::processRTCGeometry(
    const std::vector<Point3> &vertices,
    std::vector<UV> &vertexUVs,
    std::vector<Vector3> vertexNormals,
    const std::vector<FaceIndices> faces
) {
    RTCGeometry rtcMesh = rtcNewGeometry(g_rtcDevice, RTC_GEOMETRY_TYPE_TRIANGLE);
    float *rtcVertices = (float *)rtcSetNewGeometryBuffer(
        rtcMesh,                /* geometry */
        RTC_BUFFER_TYPE_VERTEX, /* type */
        0,                      /* slot */
        RTC_FORMAT_FLOAT3,      /* format */
        3 * sizeof(float),      /* byte stride */
        vertices.size()         /* item count */
    );

    int i = 0;
    for (auto &vertex : vertices) {
        rtcVertices[i * 3 + 0] = vertex.x();
        rtcVertices[i * 3 + 1] = vertex.y();
        rtcVertices[i * 3 + 2] = vertex.z();

        i += 1;
    }

    unsigned int *rtcFaces = (unsigned int *)rtcSetNewGeometryBuffer(
        rtcMesh,
        RTC_BUFFER_TYPE_INDEX,
        0,
        RTC_FORMAT_UINT3,
        3 * sizeof(unsigned int),
        faces.size()
    );

    i = 0;
    for (auto &face : faces) {
        rtcFaces[i + 0] = face.vertices[0].vertexIndex;
        rtcFaces[i + 1] = face.vertices[1].vertexIndex;
        rtcFaces[i + 2] = face.vertices[2].vertexIndex;
        i += 3;
    }

    rtcSetGeometryVertexAttributeCount(rtcMesh, 2);

    float *rtcUVs = (float *)rtcSetNewGeometryBuffer(
        rtcMesh,                          /* geometry */
        RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, /* type */
        0,                                /* slot */
        RTC_FORMAT_FLOAT2,                /* format */
        2 * sizeof(float),                /* byte stride */
        vertices.size()                   /* item count */
    );

    float *rtcNormals = (float *)rtcSetNewGeometryBuffer(
        rtcMesh,                          /* geometry */
        RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, /* type */
        1,                                /* slot */
        RTC_FORMAT_FLOAT3,                /* format */
        3 * sizeof(float),                /* byte stride */
        vertices.size()                   /* item count */
    );

    const unsigned long verticesSize = vertices.size();

    const unsigned long vertexUVsSize = vertexUVs.size();
    if (vertexUVsSize < verticesSize) {
        std::cout << "Reserving more vertex UVs (" << vertexUVsSize << ", " << verticesSize << ")" << std::endl;
        vertexUVs.resize(vertices.size(), {0.f, 0.f});
    }

    for (i = 0; i < vertices.size(); i++) {
        rtcUVs[2 * i + 0] = vertexUVs[i].u;
        rtcUVs[2 * i + 1] = vertexUVs[i].v;
    }

    const unsigned long vertexNormalsSize = vertexNormals.size();
    if (vertexNormalsSize < verticesSize) {
        std::cout << "Reserving more vertex normals (" << vertexNormalsSize << ", " << verticesSize << ")" << std::endl;
        vertexNormals.resize(vertices.size(), Vector3(0.f));
    }

    for (i = 0; i < vertices.size(); i++) {
        rtcNormals[3 * i + 0] = vertexNormals[i].x();
        rtcNormals[3 * i + 1] = vertexNormals[i].y();
        rtcNormals[3 * i + 2] = vertexNormals[i].z();
    }

    rtcCommitGeometry(rtcMesh);

    unsigned int rtcGeometryID = rtcAttachGeometry(g_rtcScene, rtcMesh);
    rtcReleaseGeometry(rtcMesh);
}
