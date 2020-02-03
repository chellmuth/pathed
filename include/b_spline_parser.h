#pragma once

#include "handedness.h"
#include "surface.h"
#include "transform.h"

#include <fstream>
#include <memory>
#include <vector>

class BSplineParser {
public:
    BSplineParser(std::ifstream &splineFile)
        : BSplineParser(splineFile, Transform(), false, Handedness::Right) {};

    BSplineParser(
        std::ifstream &splineFile,
        const Transform &transform,
        bool useFaceNormals,
        Handedness handedness
    );

    std::vector<std::shared_ptr<Surface> > parse();

private:
    std::ifstream &m_splineFile;
    Transform m_transform;
    Handedness m_handedness;
    bool m_useFaceNormals;
};