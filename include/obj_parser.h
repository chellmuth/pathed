#pragma once

#include <fstream>
#include <string>
#include <vector>

#include "mtl_parser.h"
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

    std::string m_currentGroup;

    std::vector<Point3> m_vertices;
    std::vector<std::shared_ptr<Shape>> m_faces;

    std::vector<std::shared_ptr<Model>> m_models;
    std::map<std::string, MtlMaterial> m_materialLookup;

    void parseLine(std::string &line);
    void processVertex(std::string &vertexArgs);
    void processGroup(std::string &groupArgs);
    void processFace(std::string &faceArgs);
    void processMaterialLibrary(std::string &libraryArgs);
    void processUseMaterial(std::string &materialArgs);
};

