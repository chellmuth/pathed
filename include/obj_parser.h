#pragma once

#include <fstream>
#include <string>
#include <vector>

#include "point.h"
#include "scene.h"
#include "shape.h"

enum Handedness {
    Right,
    Left
};

class ObjParser {
public:
    ObjParser(std::ifstream &objFile) : ObjParser(objFile, Handedness::Right) {};
    ObjParser(std::ifstream &objFile, Handedness handedness);
    Scene parseScene();

private:
    std::ifstream &m_objFile;
    Handedness m_handedness;

    std::vector<Point3> m_vertices;
    std::vector<Shape *> m_faces;

    void parseLine(std::string &line);
    void processVertex(std::string &vertexArgs);
    void processFace(std::string &faceArgs);
};

