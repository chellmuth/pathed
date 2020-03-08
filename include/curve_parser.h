#pragma once

#include "curve.h"
#include "surface.h"
#include "transform.h"

#include <fstream>
#include <memory>
#include <vector>

class CurveParser {
public:
    CurveParser(std::ifstream &curveFile)
        : CurveParser(curveFile, Transform(), false) {};

    CurveParser(
        std::ifstream &curveFile,
        const Transform &transform,
        bool useFaceNormals
    );

    std::vector<std::shared_ptr<Surface> > parse();

private:
    std::shared_ptr<Curve> parseCurve(const std::string &line);

    std::ifstream &m_curveFile;
    Transform m_transform;
    bool m_useFaceNormals;
};
