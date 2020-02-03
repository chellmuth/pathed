#pragma once

#include "handedness.h"
#include "surface.h"
#include "transform.h"

#include <embree3/rtcore.h>

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

    std::vector<std::vector<std::shared_ptr<Surface> > > parse(
        RTCScene rtcScene,
        float width0,
        float width1
    );

private:
    std::ifstream &m_splineFile;
    Transform m_transform;
    Handedness m_handedness;
    bool m_useFaceNormals;
};
