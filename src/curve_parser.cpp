#include "curve_parser.h"

#include "color.h"
#include "globals.h"
#include "lambertian.h"
#include "point.h"
#include "vector.h"

#include <embree3/rtcore.h>


#include <assert.h>
#include <iostream>
#include <memory>
#include <regex>
#include <stdlib.h>
#include <string>

using string = std::string;

CurveParser::CurveParser(
    std::ifstream &curveFile,
    const Transform &transform,
    bool useFaceNormals,
    Handedness handedness
) : m_curveFile(curveFile),
    m_transform(transform),
    m_useFaceNormals(useFaceNormals),
    m_handedness(handedness)
{}

std::vector<std::shared_ptr<Surface> > CurveParser::parse()
{
    std::cout << "Parsing" << std::endl;
    std::vector<std::shared_ptr<Curve> > curves;

    string line;
    std::getline(m_curveFile, line);
    assert(line == "Shape \"curve\" \"string type\" \"cylinder\" \"point P\" [ 0.005968 0.095212 -0.003707 0.006467 0.099219 -0.005693 0.007117 0.102842 -0.008281 0.007995 0.105534 -0.011779 ] \"float width0\" 0.000180 \"float width1\" 0.000015");
    std::cout << line << std::endl;
    curves.push_back(parseCurve(line));

    while(std::getline(m_curveFile, line)) {
        curves.push_back(parseCurve(line));
    }

    std::cout << "Creating RTC resources" << std::endl;

    RTCGeometry rtcMesh = rtcNewGeometry(g_rtcDevice, RTC_GEOMETRY_TYPE_ROUND_BEZIER_CURVE);

    float *rtcVertices = (float *)rtcSetNewGeometryBuffer(
        rtcMesh,                /* geometry */
        RTC_BUFFER_TYPE_VERTEX, /* type */
        0,                      /* slot */
        RTC_FORMAT_FLOAT4,      /* format */
        4 * sizeof(float),      /* byte stride */
        curves.size() * 4       /* item count */
    );

    unsigned int *rtcIndices = (unsigned int *)rtcSetNewGeometryBuffer(
        rtcMesh,
        RTC_BUFFER_TYPE_INDEX,
        0,
        RTC_FORMAT_UINT,
        1 * sizeof(unsigned int),
        curves.size()
    );

    for (int i = 0; i < curves.size(); i++) {
        const auto &curvePtr = curves[i];

        rtcVertices[16 * i + 0] = curvePtr->p1().x();
        rtcVertices[16 * i + 1] = curvePtr->p1().y();
        rtcVertices[16 * i + 2] = curvePtr->p1().z();
        rtcVertices[16 * i + 3] = curvePtr->width0();

        rtcVertices[16 * i + 4 + 0] = curvePtr->p2().x();
        rtcVertices[16 * i + 4 + 1] = curvePtr->p2().y();
        rtcVertices[16 * i + 4 + 2] = curvePtr->p2().z();
        rtcVertices[16 * i + 4 + 3] = curvePtr->width0();

        rtcVertices[16 * i + 8 + 0] = curvePtr->p3().x();
        rtcVertices[16 * i + 8 + 1] = curvePtr->p3().y();
        rtcVertices[16 * i + 8 + 2] = curvePtr->p3().z();
        rtcVertices[16 * i + 8 + 3] = curvePtr->width1();

        rtcVertices[16 * i + 12 + 0] = curvePtr->p4().x();
        rtcVertices[16 * i + 12 + 1] = curvePtr->p4().y();
        rtcVertices[16 * i + 12 + 2] = curvePtr->p4().z();
        rtcVertices[16 * i + 12 + 3] = curvePtr->width1();

        rtcIndices[i] = i * 4;
    }

    rtcCommitGeometry(rtcMesh);

    unsigned int rtcGeometryID = rtcAttachGeometry(g_rtcScene, rtcMesh);
    rtcReleaseGeometry(rtcMesh);

    std::cout << "Creating internal surfaces" << std::endl;

    std::vector<std::shared_ptr<Surface> > surfaces;
    for (auto curvePtr : curves) {
        Transform identity;
        auto materialPtr = std::make_shared<Lambertian>(Color(1.f, 0.f, 0.f), Color(0.f));
        auto surfacePtr = std::make_shared<Surface>(curvePtr, materialPtr, nullptr);
        surfaces.push_back(surfacePtr);
    }

    std::cout << "Parsing complete!" << std::endl;

    return surfaces;
}

std::shared_ptr<Curve> CurveParser::parseCurve(const std::string &line)
{
    static const std::regex lineExpression("Shape \"curve\" \"string type\" \"cylinder\" \"point P\" \\[ (-?\\d+\\.\\d+) (-?\\d+\\.\\d+) (-?\\d+\\.\\d+) (-?\\d+\\.\\d+) (-?\\d+\\.\\d+) (-?\\d+\\.\\d+) (-?\\d+\\.\\d+) (-?\\d+\\.\\d+) (-?\\d+\\.\\d+) (-?\\d+\\.\\d+) (-?\\d+\\.\\d+) (-?\\d+\\.\\d+) \\] \"float width0\" (-?\\d+\\.\\d+) \"float width1\" (-?\\d+\\.\\d+)");
    std::smatch lineMatch;
    std::regex_match(line, lineMatch, lineExpression);
    assert(!lineMatch.empty());

    auto curvePtr = std::make_shared<Curve>(
        Point3(
            strtof(lineMatch[1].str().c_str(), NULL),
            strtof(lineMatch[2].str().c_str(), NULL),
            strtof(lineMatch[3].str().c_str(), NULL)
        ),
        Point3(
            strtof(lineMatch[4].str().c_str(), NULL),
            strtof(lineMatch[5].str().c_str(), NULL),
            strtof(lineMatch[6].str().c_str(), NULL)
        ),
        Point3(
            strtof(lineMatch[7].str().c_str(), NULL),
            strtof(lineMatch[8].str().c_str(), NULL),
            strtof(lineMatch[9].str().c_str(), NULL)
        ),
        Point3(
            strtof(lineMatch[10].str().c_str(), NULL),
            strtof(lineMatch[11].str().c_str(), NULL),
            strtof(lineMatch[12].str().c_str(), NULL)
        ),
        strtof(lineMatch[13].str().c_str(), NULL),
        strtof(lineMatch[14].str().c_str(), NULL)
    );

    return curvePtr;
}
