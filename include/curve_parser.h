#pragma once

#include "handedness.h"
#include "surface.h"
#include "transform.h"

#include <fstream>
#include <memory>
#include <vector>

class CurveParser {
public:
    CurveParser(std::ifstream &curveFile)
        : CurveParser(curveFile, Transform(), false, Handedness::Right) {};

    CurveParser(
        std::ifstream &curveFile,
        const Transform &transform,
        bool useFaceNormals,
        Handedness handedness
    );

    void parse(std::vector<std::vector<std::shared_ptr<Surface> > > &surfaces);

private:
    std::vector<std::shared_ptr<Surface> > parseCurve(const std::string &line);

    std::ifstream &m_curveFile;
    Transform m_transform;
    Handedness m_handedness;
    bool m_useFaceNormals;
};
