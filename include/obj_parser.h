#pragma once

#include "blank_shape.h"
#include "geometry_parser.h"
#include "globals.h"
#include "material.h"
#include "mtl_parser.h"
#include "light.h"
#include "point.h"
#include "surface.h"
#include "transform.h"
#include "uv.h"

#include <embree3/rtcore.h>

#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

class Triangle;

class ObjParser {
public:
    ObjParser(
        std::ifstream &objFile,
        const Transform &transform,
        bool useFaceNormals,
        RTCScene rtcScene,
        std::map<std::string, std::shared_ptr<Material> > materialLookup,
        std::string &materialPrefix
    );

    std::vector<std::shared_ptr<Surface> > parse();

private:
    std::ifstream &m_objFile;
    Transform m_transform;
    bool m_useFaceNormals;
    RTCScene m_rtcScene;

    std::string m_currentGroup;
    std::string m_currentMaterialName;
    int m_currentFaceIndex;

    std::vector<Point3> m_vertices;
    std::vector<unsigned int> m_faces;
    std::vector<Vector3> m_normals;
    std::vector<UV> m_uvs;
    std::vector<FaceIndices> m_faceIndices;

    std::vector<Vector3> m_vertexNormals;
    std::vector<UV> m_vertexUVs;

    std::vector<std::shared_ptr<Surface>> m_surfaces;
    std::vector<std::shared_ptr<Light>> m_lights;

    std::map<std::string, std::shared_ptr<Material> > m_materialLookup;
    std::map<std::string, std::shared_ptr<Material> > m_mtlLookup;
    std::string m_materialPrefix;
    std::shared_ptr<Material> m_defaultMaterialPtr;

    std::shared_ptr<BlankTriangle> m_defaultShapePtr;

    void parseLine(std::string &line);
    void processVertex(std::string &vertexArgs);
    void processNormal(std::string &vertexArgs);
    void processUV(std::string &UVArgs);
    void processGroup(std::string &groupArgs);
    void processFace(std::string &faceArgs);
    void processMaterialLibrary(std::string &libraryArgs);
    void processUseMaterial(std::string &materialArgs);

    void processFace(Triangle *face);

    void processTriangle(
        int vertexIndex0, int vertexIndex1, int vertexIndex2,
        int normalIndex0, int normalIndex1, int normalIndex2,
        int UVIndex0, int UVIndex1, int UVIndex2
    );
    void processTriangle(
        int vertexIndex0, int vertexIndex1, int vertexIndex2,
        int normalIndex0, int normalIndex1, int normalIndex2
    );
    void processTriangle(int vertexIndex0, int vertexIndex1, int vertexIndex2);

    bool processDoubleFaceGeometryOnly(std::string &faceArgs);
    bool processSingleFaceTripletsVertexAndNormal(std::string &faceArgs);
    bool processDoubleFaceTripletsVertexAndNormal(std::string &faceArgs);
    bool processSingleFaceTriplets(std::string &faceArgs);
};
