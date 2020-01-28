#include "curve_parser.h"

#include "color.h"
#include "curve.h"
#include "lambertian.h"
#include "point.h"
#include "vector.h"

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

using NestedSurfaceVector = std::vector<std::vector<std::shared_ptr<Surface> > >;

void CurveParser::parse(NestedSurfaceVector &surfaces)
{
    string line;
    std::getline(m_curveFile, line);
    assert(line == "Shape \"curve\" \"string type\" \"cylinder\" \"point P\" [ 0.005968 0.095212 -0.003707 0.006467 0.099219 -0.005693 0.007117 0.102842 -0.008281 0.007995 0.105534 -0.011779 ] \"float width0\" 0.000180 \"float width1\" 0.000015");
    std::cout << line << std::endl;

    std::vector<std::shared_ptr<Surface> > curveSurfaces;

    surfaces.push_back(parseCurve(line));

    while(std::getline(m_curveFile, line)) {
        // std::cout << line << std::endl;
        surfaces.push_back(parseCurve(line));
    }
}

std::vector<std::shared_ptr<Surface> > CurveParser::parseCurve(const std::string &line)
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

    Transform identity;
    auto materialPtr = std::make_shared<Lambertian>(Color(1.f, 0.f, 0.f), Color(0.f));
    curvePtr->create(
        identity,
        materialPtr
    );

    auto surfacePtr = std::make_shared<Surface>(curvePtr, materialPtr, nullptr);
    std::vector<std::shared_ptr<Surface> > curveSurfaces;
    curveSurfaces.push_back(surfacePtr);
    return curveSurfaces;
}
