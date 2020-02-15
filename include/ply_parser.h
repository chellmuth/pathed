#pragma once

#include "surface.h"
#include "transform.h"

#include <fstream>
#include <memory>
#include <vector>

class PLYParser {
public:
    PLYParser(std::ifstream &plyFile)
        : PLYParser(plyFile, Transform(), false) {};

    PLYParser(
        std::ifstream &objFile,
        const Transform &transform,
        bool useFaceNormals
    );

    std::vector<std::shared_ptr<Surface> > parse();

private:
    std::ifstream &m_objFile;
    Transform m_transform;
    bool m_useFaceNormals;
};
