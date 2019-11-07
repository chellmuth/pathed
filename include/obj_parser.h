#pragma once

#include "mtl_parser.h"
#include "light.h"
#include "point.h"
#include "surface.h"
#include "transform.h"
#include "uv.h"

#include <embree3/rtcore.h>

#include <fstream>
#include <string>
#include <vector>

enum class Handedness {
    Right,
    Left
};

class ObjParser {
public:
    ObjParser(std::ifstream &objFile) : ObjParser(objFile, Transform(), false, Handedness::Right) {};
    ObjParser(std::ifstream &objFile, bool useFaceNormals) : ObjParser(objFile, Transform(), useFaceNormals, Handedness::Right) {};
    ObjParser(std::ifstream &objFile, const Transform &transform, bool useFaceNormals, Handedness handedness);

    std::vector<std::shared_ptr<Surface> > parse();

private:
    std::ifstream &m_objFile;
    Transform m_transform;
    Handedness m_handedness;
    bool m_useFaceNormals;

    RTCGeometry m_geometry;

    std::string m_currentGroup;
    std::string m_currentMaterialName;

    std::vector<Point3> m_vertices;
    std::vector<unsigned int> m_faces;
    std::vector<Vector3> m_normals;
    std::vector<UV> m_uvs;
    std::vector<std::shared_ptr<Surface>> m_surfaces;
    std::vector<std::shared_ptr<Light>> m_lights;

    std::map<std::string, MtlMaterial> m_materialLookup;

    void parseLine(std::string &line);
    void processVertex(std::string &vertexArgs);
    void processNormal(std::string &vertexArgs);
    void processUV(std::string &UVArgs);
    void processGroup(std::string &groupArgs);
    void processFace(std::string &faceArgs);
    void processMaterialLibrary(std::string &libraryArgs);
    void processUseMaterial(std::string &materialArgs);

    void processFace(Triangle *face);
    void processTriangle(int vertexIndex0, int vertexIndex1, int vertexIndex2);
    void processTriangle(
        int vertexIndex0, int vertexIndex1, int vertexIndex2,
        int UVIndex0, int UVIndex1, int UVIndex2
    );

    bool processDoubleFaceGeometryOnly(std::string &faceArgs);
    bool processSingleFaceTripletsVertexAndNormal(std::string &faceArgs);
    bool processSingleFaceTriplets(std::string &faceArgs);
};
