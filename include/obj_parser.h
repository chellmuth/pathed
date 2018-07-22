#pragma once

#include <fstream>
#include <string>
#include <vector>

#include "point.h"
#include "scene.h"
#include "shape.h"

class ObjParser {
public:
    Scene parseScene(std::ifstream &objFile);

private:
    std::vector<Point3> m_vertices;
    std::vector<Shape *> m_faces;

    void parseLine(std::string &line);
    void processVertex(std::string &vertexArgs);
    void processFace(std::string &faceArgs);
};

