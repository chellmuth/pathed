#include "b_spline_parser.h"

#include "b_spline.h"
#include "color.h"
#include "lambertian.h"
#include "vector.h"

#include "json.hpp"
using json = nlohmann::json;

#include <iostream>

BSplineParser::BSplineParser(
    std::ifstream &splineFile,
    const Transform &transform,
    bool useFaceNormals,
    Handedness handedness
) : m_splineFile(splineFile),
    m_transform(transform),
    m_useFaceNormals(useFaceNormals),
    m_handedness(handedness)
{}

std::vector<std::vector<std::shared_ptr<Surface> > > BSplineParser::parse(RTCScene rtcScene)
{
    std::cout << "Parsing" << std::endl;
    json splineJson = json::parse(m_splineFile);

    std::cout << "Creating internal surfaces" << std::endl;
    Transform identity;
    std::vector<std::shared_ptr<Surface> > surfaces;
    auto materialPtr = std::make_shared<Lambertian>(Color(1.f, 0.f, 0.f), Color(0.f));
    for (auto &spline : splineJson) {
        std::vector<Point3> points;
        for (auto &pointList : spline) {
            points.push_back(Point3(pointList[0], pointList[1], pointList[2]));
        }

        auto splinePtr = std::make_shared<BSpline>(
            points,
            1.f,
            1.f
        );
        auto surfacePtr = std::make_shared<Surface>(splinePtr, materialPtr, nullptr);
        surfaces.push_back({surfacePtr});
    }


    std::cout << "Creating RTC resources" << std::endl;
    for (auto &surfacePtr : surfaces) {
        auto shapePtr = surfacePtr->getShape();
        dynamic_cast<BSpline *>(shapePtr.get())->create(rtcScene);
    }

    std::cout << "Creating nested surfaces" << std::endl;
    std::vector<std::vector<std::shared_ptr<Surface> > > nestedSurfaces;
    for (auto &surfacePtr : surfaces) {
        nestedSurfaces.push_back({surfacePtr, surfacePtr, surfacePtr, surfacePtr});
    }

    std::cout << "Parsing complete!" << std::endl;

    return nestedSurfaces;
}
