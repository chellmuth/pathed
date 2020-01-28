#pragma once

#include "handedness.h"
#include "surface.h"
#include "transform.h"

#include <fstream>
#include <memory>
#include <vector>

class PLYParser {
public:
    PLYParser(std::ifstream &plyFile)
        : PLYParser(plyFile, Transform(), false, Handedness::Right) {};

    PLYParser(
        std::ifstream &objFile,
        const Transform &transform,
        bool useFaceNormals,
        Handedness handedness
    );

    std::vector<std::shared_ptr<Surface> > parse();

private:
    std::ifstream &m_objFile;
    Transform m_transform;
    Handedness m_handedness;
    bool m_useFaceNormals;
};
