#include "b_spline_parser.h"

#include "b_spline.h"
#include "blank_shape.h"
#include "color.h"
#include "globals.h"
#include "lambertian.h"
#include "vector.h"

#include "json.hpp"
using json = nlohmann::json;

#include <iostream>

BSplineParser::BSplineParser(
    std::ifstream &splineFile,
    std::shared_ptr<Material> materialPtr,
    const Transform &transform,
    bool useFaceNormals,
    Handedness handedness
) : m_splineFile(splineFile),
    m_materialPtr(materialPtr),
    m_transform(transform),
    m_useFaceNormals(useFaceNormals),
    m_handedness(handedness)
{}

std::vector<std::vector<std::shared_ptr<Surface> > > BSplineParser::parse(
    RTCScene rtcScene,
    float width0,
    float width1
) {
    if (!m_materialPtr) {
        m_materialPtr = std::make_shared<Lambertian>(
            Color(1.f, 0.f, 0.f),
            Color(0.f)
        );
    }

    std::cout << "Parsing" << std::endl;
    json splineJson = json::parse(m_splineFile);

    std::cout << "Creating internal surfaces" << std::endl;
    Transform identity;
    std::vector<std::shared_ptr<Surface> > surfaces;
    std::vector<std::shared_ptr<Surface> > surfacesDuped;

    for (auto &spline : splineJson) {
        std::vector<Point3> points;
        for (auto &pointList : spline) {
            points.push_back(Point3(pointList[0], pointList[1], pointList[2]));
        }

        auto splinePtr = std::make_shared<BSpline>(
            points,
            width0,
            width1
        );
        auto surfacePtr = std::make_shared<Surface>(splinePtr, m_materialPtr, nullptr);
        surfaces.push_back({surfacePtr});

        for (int i = 0; i < points.size() - 3; i++) {
            surfacesDuped.push_back({
                std::make_shared<Surface>(
                    std::make_shared<BlankSpline>(),
                    m_materialPtr,
                    nullptr
                )
            });
        }
    }

    std::cout << "Creating RTC resources" << std::endl;
    createRTCGeometry(rtcScene, surfaces);

    std::cout << "Creating nested surfaces" << std::endl;
    std::vector<std::vector<std::shared_ptr<Surface> > > nestedSurfaces;
    nestedSurfaces.push_back(surfacesDuped);

    std::cout << "Parsing complete!" << std::endl;

    return nestedSurfaces;
}

void BSplineParser::createRTCGeometry(
    RTCScene rtcScene,
    std::vector<std::shared_ptr<Surface> > &splines
) {
    RTCGeometry rtcMesh = rtcNewGeometry(g_rtcDevice, RTC_GEOMETRY_TYPE_FLAT_BSPLINE_CURVE);

    size_t pointsCount = 0;
    size_t indicesCount = 0;
    for (auto &surfacePtr : splines) {
        auto shapePtr = surfacePtr->getShape();
        auto &points = dynamic_cast<BSpline *>(shapePtr.get())->points();

        pointsCount += points.size();
        indicesCount += points.size() - 3;
    }

    float *rtcVertices = (float *)rtcSetNewGeometryBuffer(
        rtcMesh,                /* geometry */
        RTC_BUFFER_TYPE_VERTEX, /* type */
        0,                      /* slot */
        RTC_FORMAT_FLOAT4,      /* format */
        4 * sizeof(float),      /* byte stride */
        pointsCount             /* item count */
    );

    unsigned int *rtcIndices = (unsigned int *)rtcSetNewGeometryBuffer(
        rtcMesh,
        RTC_BUFFER_TYPE_INDEX,
        0,
        RTC_FORMAT_UINT,
        1 * sizeof(unsigned int),
        indicesCount
    );

    int vertexCount = 0;
    int indexCount = 0;

    for (auto &surfacePtr : splines) {
        auto shapePtr = surfacePtr->getShape();
        BSpline *spline = dynamic_cast<BSpline *>(shapePtr.get());
        const std::vector<Point3> &points = spline->points();

        const size_t count = points.size();
        for (int i = 0; i < count; i++) {
            const Point3 &point = points[i];
            rtcVertices[(vertexCount + i) * 4 + 0] = point.x();
            rtcVertices[(vertexCount + i) * 4 + 1] = point.y();
            rtcVertices[(vertexCount + i) * 4 + 2] = point.z();

            const float alpha = (1.f * i) / (count - 1);
            rtcVertices[(vertexCount + i) * 4 + 3] = spline->width0() * (1.f - alpha) + spline->width1() * alpha;
        }

        for (int i = 0; i < count - 3; i++) {
            rtcIndices[indexCount + i] = vertexCount + i;
        }

        vertexCount += count;
        indexCount += count - 3;
    }

    rtcCommitGeometry(rtcMesh);

    unsigned int rtcGeometryID = rtcAttachGeometry(rtcScene, rtcMesh);
    rtcReleaseGeometry(rtcMesh);
}
