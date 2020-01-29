#pragma once

#include "point.h"
#include "uv.h"
#include "vector.h"

#include <embree3/rtcore.h>

#include <vector>

struct FaceIndices {
    struct VertexIndices {
        int vertexIndex;
        int normalIndex;
        int UVIndex;
    };
    VertexIndices vertices[3];
};

namespace GeometryParser {
    void processRTCGeometry(
        RTCScene rtcScene,
        const std::vector<Point3> &vertices,
        std::vector<UV> &vertexUVs,
        std::vector<Vector3> vertexNormals,
        const std::vector<FaceIndices> faces
    );
};
