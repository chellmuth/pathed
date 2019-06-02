#pragma once

#include <fstream>
#include <string>
#include <vector>

#include "mtl_parser.h"
#include "light.h"
#include "point.h"
#include "scene.h"
#include "surface.h"

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
    std::string m_currentMaterialName;

    std::vector<Point3> m_vertices;
    std::vector<std::shared_ptr<Surface>> m_surfaces;
    std::vector<std::shared_ptr<Light>> m_lights;

    std::map<std::string, MtlMaterial> m_materialLookup;

    void parseLine(std::string &line);
    void processVertex(std::string &vertexArgs);
    void processGroup(std::string &groupArgs);
    void processFace(std::string &faceArgs);
    void processMaterialLibrary(std::string &libraryArgs);
    void processUseMaterial(std::string &materialArgs);

    bool processDoubleFaceGeometryOnly(std::string &faceArgs);
    bool processSingleFaceTriplets(std::string &faceArgs);
};

