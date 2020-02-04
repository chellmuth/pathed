#pragma once

#include "handedness.h"
#include "material.h"
#include "surface.h"
#include "transform.h"

#include <embree3/rtcore.h>

#include <fstream>
#include <memory>
#include <vector>

class BSplineParser {
public:
    BSplineParser(
        std::ifstream &splineFile,
        std::shared_ptr<Material> materialPtr,
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
    void createRTCGeometry(
        RTCScene rtcScene,
        std::vector<std::shared_ptr<Surface> > &splines
    );

    std::ifstream &m_splineFile;
    std::shared_ptr<Material> m_materialPtr;
    Transform m_transform;
    Handedness m_handedness;
    bool m_useFaceNormals;
};
