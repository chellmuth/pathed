#include "obj_parser.h"

#include "area_light.h"
#include "blank_shape.h"
#include "camera.h"
#include "color.h"
#include "lambertian.h"
#include "primitive.h"
#include "string_util.h"
#include "triangle.h"
#include "vector.h"

#include <iostream>
#include <map>
#include <memory>
#include <regex>
#include <utility>
#include <vector>

using string = std::string;

ObjParser::ObjParser(
    std::ifstream &objFile,
    const Transform &transform,
    bool useFaceNormals,
    Handedness handedness,
    RTCScene rtcScene,
    std::map<std::string, std::shared_ptr<Material> > materialLookup,
    string &materialPrefix
)
    : m_objFile(objFile),
      m_transform(transform),
      m_useFaceNormals(useFaceNormals),
      m_handedness(handedness),
      m_rtcScene(rtcScene),
      m_currentGroup(""),
      m_materialLookup(materialLookup),
      m_materialPrefix(materialPrefix)
{
    m_defaultMaterialPtr = std::make_shared<Lambertian>(
        Color(1.f, 0.f, 0.f),
        Color(0.f)
    );

    m_defaultShapePtr = std::make_shared<BlankShape>();
}

std::vector<std::shared_ptr<Surface> > ObjParser::parse()
{
    string line;
    while(std::getline(m_objFile, line)) {
        parseLine(line);
    }

    std::map<int, int> normalLookup;
    std::map<std::pair<int, int>, int> correctionLookup;

    // Start "cube-normal" correction
    // Look for re-used vertices with differing normals
    // When found, create a new vertex (add to the back of m_vertices)
    // In the next loop, update m_vertexNormals
    //   * reverse overwritten normals
    //   * associate new normals with created vertices
    for (int i = 0; i < m_faceIndices.size(); i++) {
        FaceIndices &faceIndices = m_faceIndices[i];
        FaceIndices correctedFace;

        for (int j = 0; j < 3; j++) {
            FaceIndices::VertexIndices &vertexIndices = faceIndices.vertices[j];

            if (normalLookup.count(vertexIndices.vertexIndex) == 0) {
                normalLookup[vertexIndices.vertexIndex] = vertexIndices.normalIndex;

                correctedFace.vertices[j] = vertexIndices;
            } else if (normalLookup[vertexIndices.vertexIndex] != vertexIndices.normalIndex) {
                std::pair<int, int> key(vertexIndices.vertexIndex, vertexIndices.normalIndex);
                int correctedIndex;
                if (correctionLookup.count(key) == 0) {
                    m_vertices.push_back(m_vertices[vertexIndices.vertexIndex]);

                    correctedIndex = m_vertices.size() - 1;
                    correctionLookup[key] = correctedIndex;
                } else {
                    correctedIndex = correctionLookup[key];
                }

                correctedFace.vertices[j] = {
                    .vertexIndex = correctedIndex,
                    .normalIndex = vertexIndices.normalIndex,
                    .UVIndex = vertexIndices.UVIndex
                };
            } else {
                correctedFace.vertices[j] = vertexIndices;
            }
        }

        m_faceIndices[i] = correctedFace;
        // correctedFaces.push_back(correctedFace);
    }

    std::vector<FaceIndices> &correctedFaces = m_faceIndices;

    m_vertexNormals.resize(m_vertices.size(), Vector3(0.f));
    for (int i = 0; i < correctedFaces.size(); i++) {
        FaceIndices &faceIndices = correctedFaces[i];

        for (int j = 0; j < 3; j++) {
            FaceIndices::VertexIndices &vertexIndices = faceIndices.vertices[j];
            int normalIndex = vertexIndices.normalIndex;
            if (normalIndex != -1) {
                m_vertexNormals[vertexIndices.vertexIndex] = m_normals[normalIndex];
            }
        }
    }
    // End "cube-normal" correction

    GeometryParser::processRTCGeometry(
        m_rtcScene,
        m_vertices,
        m_vertexUVs,
        m_vertexNormals,
        correctedFaces
    );

    return m_surfaces;
}

void ObjParser::parseLine(string &line)
{
    if (line.empty()) { return; }

    string::size_type spaceIndex = line.find_first_of(" \t");
    if (spaceIndex == string::npos) { return; }

    string command = line.substr(0, spaceIndex);
    if (command[0] == '#') { return; }

    string rest = lTrim(line.substr(spaceIndex + 1));

    if (command == "v") {
        processVertex(rest);
    } else if (command == "vn") {
        processNormal(rest);
    } else if (command == "vt") {
        processUV(rest);
    } else if (command == "g") {
        processGroup(rest);
    } else if (command == "f") {
        if (m_currentMaterialName == "hidden") {
            return;
        }

        processFace(rest);
    } else if (command == "mtllib") {
        processMaterialLibrary(rest);
    } else if (command == "usemtl") {
        processUseMaterial(rest);
    }
}

void ObjParser::processVertex(string &vertexArgs)
{
    string::size_type index = 0;
    string rest = vertexArgs;

    float x = std::stof(rest, &index);
    if (m_handedness == Handedness::Left) {
        x *= -1.f;
    }

    rest = rest.substr(index);
    float y = std::stof(rest, &index);

    rest = rest.substr(index);
    float z = std::stof(rest, &index);

    Point3 vertex = m_transform.apply(Point3(x, y, z));
    m_vertices.push_back(vertex);
}

void ObjParser::processNormal(string &normalArgs)
{
    string::size_type index = 0;
    string rest = normalArgs;

    float x = std::stof(rest, &index);
    if (m_handedness == Handedness::Left) {
        x *= -1.f;
    }

    rest = rest.substr(index);
    float y = std::stof(rest, &index);

    rest = rest.substr(index);
    float z = std::stof(rest, &index);

    Vector3 normal(x, y, z);
    m_normals.push_back(m_transform.apply(normal));
}

void ObjParser::processUV(string &uvArgs)
{
    string::size_type index = 0;
    string rest = uvArgs;

    float u = std::stof(rest, &index);

    rest = rest.substr(index);
    float v = std::stof(rest, &index);
    if (m_handedness == Handedness::Left) {
        v = 1 - v;
    }

    UV uv = { u, v };
    m_uvs.push_back(uv);
}

void ObjParser::processGroup(string &groupArgs)
{
    string name = lTrim(groupArgs);
    m_currentGroup = name;
    m_currentFaceIndex = 0;
}

template <class T>
static void correctIndex(const std::vector<T> &indices, int *index) {
    if (*index < 0) {
        *index += indices.size();
    } else {
        *index -= 1;
    }
}

template <class T>
static void correctIndices(
    const std::vector<T> &indices,
    int *index0,
    int *index1,
    int *index2
) {
    correctIndex(indices, index0);
    correctIndex(indices, index1);
    correctIndex(indices, index2);
}

void ObjParser::processFace(Triangle *face)
{
    std::shared_ptr<Material> materialPtr;
    std::string materialGroupKey = m_materialPrefix + m_currentGroup;
    std::string materialMtlKey = m_materialPrefix + m_currentMaterialName;
    if (m_materialLookup.count(materialGroupKey) > 0) {
        materialPtr = m_materialLookup.at(materialGroupKey);
    } else if (m_materialLookup.count(materialMtlKey) > 0) {
        materialPtr = m_materialLookup.at(materialMtlKey);
    } else if (m_mtlLookup.count(m_currentMaterialName) > 0) {
        materialPtr = m_mtlLookup.at(m_currentMaterialName);
    } else {
        materialPtr = m_defaultMaterialPtr;
    }

    std::shared_ptr<Surface> surface = std::make_shared<Surface>(
        m_defaultShapePtr,
        materialPtr,
        nullptr,
        m_currentFaceIndex
    );

    m_surfaces.push_back(surface);
    m_currentFaceIndex += 1;

    if (materialPtr && materialPtr->emit().isBlack()) { return; }

    std::shared_ptr<Light> light(new AreaLight(surface));

    m_lights.push_back(light);
}

void ObjParser::processTriangle(
    int vertexIndex0, int vertexIndex1, int vertexIndex2,
    int normalIndex0, int normalIndex1, int normalIndex2,
    int UVIndex0, int UVIndex1, int UVIndex2
) {
    correctIndices(m_vertices, &vertexIndex0, &vertexIndex1, &vertexIndex2);
    correctIndices(m_normals, &normalIndex0, &normalIndex1, &normalIndex2);
    correctIndices(m_uvs, &UVIndex0, &UVIndex1, &UVIndex2);

    // Triangle *face = new Triangle(
    //     m_vertices[vertexIndex0],
    //     m_vertices[vertexIndex1],
    //     m_vertices[vertexIndex2]
    // );

    processFace(nullptr);

    m_faces.push_back(vertexIndex0);
    m_faces.push_back(vertexIndex1);
    m_faces.push_back(vertexIndex2);

    m_vertexUVs.resize(m_vertices.size(), {0.f, 0.f});
    m_vertexUVs[vertexIndex0] = m_uvs[UVIndex0];
    m_vertexUVs[vertexIndex1] = m_uvs[UVIndex1];
    m_vertexUVs[vertexIndex2] = m_uvs[UVIndex2];

    m_vertexNormals.resize(m_vertices.size(), Vector3(0.f));
    m_vertexNormals[vertexIndex0] = m_normals[normalIndex0];
    m_vertexNormals[vertexIndex1] = m_normals[normalIndex1];
    m_vertexNormals[vertexIndex2] = m_normals[normalIndex2];

    m_faceIndices.push_back({
        .vertices = {
            { vertexIndex0, normalIndex0, UVIndex0 },
            { vertexIndex1, normalIndex1, UVIndex1 },
            { vertexIndex2, normalIndex2, UVIndex2 }
        }
    });
}

void ObjParser::processTriangle(
    int vertexIndex0, int vertexIndex1, int vertexIndex2,
    int normalIndex0, int normalIndex1, int normalIndex2
) {

    correctIndices(m_vertices, &vertexIndex0, &vertexIndex1, &vertexIndex2);
    correctIndices(m_normals, &normalIndex0, &normalIndex1, &normalIndex2);

    // Triangle *face = new Triangle(
    //     m_vertices[vertexIndex0],
    //     m_vertices[vertexIndex1],
    //     m_vertices[vertexIndex2]
    // );

    processFace(nullptr);

    m_faces.push_back(vertexIndex0);
    m_faces.push_back(vertexIndex1);
    m_faces.push_back(vertexIndex2);

    m_vertexNormals.resize(m_vertices.size(), Vector3(0.f));
    m_vertexNormals[vertexIndex0] = m_normals[normalIndex0];
    m_vertexNormals[vertexIndex1] = m_normals[normalIndex1];
    m_vertexNormals[vertexIndex2] = m_normals[normalIndex2];

    m_faceIndices.push_back({
        .vertices = {
            { vertexIndex0, normalIndex0, -1 },
            { vertexIndex1, normalIndex1, -1 },
            { vertexIndex2, normalIndex2, -1 }
        }
    });
}

void ObjParser::processTriangle(int vertexIndex0, int vertexIndex1, int vertexIndex2)
{
    correctIndices(m_vertices, &vertexIndex0, &vertexIndex1, &vertexIndex2);

    // Triangle *face = new Triangle(
    //     m_vertices[vertexIndex0],
    //     m_vertices[vertexIndex1],
    //     m_vertices[vertexIndex2]
    // );

    processFace(nullptr);

    m_faces.push_back(vertexIndex0);
    m_faces.push_back(vertexIndex1);
    m_faces.push_back(vertexIndex2);

    m_faceIndices.push_back({
        .vertices = {
            { vertexIndex0, -1, -1 },
            { vertexIndex1, -1, -1 },
            { vertexIndex2, -1, -1 }
        }
    });
}

bool ObjParser::processDoubleFaceGeometryOnly(std::string &faceArgs)
{
    static const std::regex expression("(-?\\d+)\\s+(-?\\d+)\\s+(-?\\d+)\\s+(-?\\d+)\\s*");
    std::smatch match;
    std::regex_match(faceArgs, match, expression);

    if (match.empty()) {
        return false;
    }

    int index0 = std::stoi(match[1]);
    int index1 = std::stoi(match[2]);
    int index2 = std::stoi(match[3]);
    int index3 = std::stoi(match[4]);

    processTriangle(index0, index1, index2);
    processTriangle(index0, index2, index3);

    return true;
}

bool ObjParser::processSingleFaceTriplets(std::string &faceArgs)
{
    static std::regex expression("(-?\\d+)/(-?\\d+)/(-?\\d+) (-?\\d+)/(-?\\d+)/(-?\\d+) (-?\\d+)/(-?\\d+)/(-?\\d+)\\s*");
    std::smatch match;
    std::regex_match (faceArgs, match, expression);

    if (match.empty()) {
        return false;
    }

    int vertexIndex0 = std::stoi(match[1]);
    int vertexIndex1 = std::stoi(match[4]);
    int vertexIndex2 = std::stoi(match[7]);

    int UVIndex0 = std::stoi(match[2]);
    int UVIndex1 = std::stoi(match[5]);
    int UVIndex2 = std::stoi(match[8]);

    int normalIndex0 = std::stoi(match[3]);
    int normalIndex1 = std::stoi(match[6]);
    int normalIndex2 = std::stoi(match[9]);

    processTriangle(
        vertexIndex0, vertexIndex1, vertexIndex2,
        normalIndex0, normalIndex1, normalIndex2,
        UVIndex0, UVIndex1, UVIndex2
    );

    return true;
}

bool ObjParser::processSingleFaceTripletsVertexAndNormal(std::string &faceArgs)
{
    static std::regex expression("(-?\\d+)//(-?\\d+)\\s+(-?\\d+)//(-?\\d+)\\s+(-?\\d+)//(-?\\d+)\\s*");
    std::smatch match;
    std::regex_match (faceArgs, match, expression);

    if (match.empty()) {
        return false;
    }

    int vertexIndex0 = std::stoi(match[1]);
    int vertexIndex1 = std::stoi(match[3]);
    int vertexIndex2 = std::stoi(match[5]);

    int normalIndex0 = std::stoi(match[2]);
    int normalIndex1 = std::stoi(match[4]);
    int normalIndex2 = std::stoi(match[6]);

    processTriangle(
        vertexIndex0, vertexIndex1, vertexIndex2,
        normalIndex0, normalIndex1, normalIndex2
    );

    return true;
}

bool ObjParser::processDoubleFaceTripletsVertexAndNormal(std::string &faceArgs)
{
    static std::regex expression("(-?\\d+)//(-?\\d+) (-?\\d+)//(-?\\d+) (-?\\d+)//(-?\\d+) (-?\\d+)//(-?\\d+)\\s*");
    std::smatch match;
    std::regex_match (faceArgs, match, expression);

    if (match.empty()) {
        return false;
    }

    int vertexIndex0 = std::stoi(match[1]);
    int vertexIndex1 = std::stoi(match[3]);
    int vertexIndex2 = std::stoi(match[5]);
    int vertexIndex3 = std::stoi(match[7]);

    int normalIndex0 = std::stoi(match[2]);
    int normalIndex1 = std::stoi(match[4]);
    int normalIndex2 = std::stoi(match[6]);
    int normalIndex3 = std::stoi(match[8]);

    processTriangle(
        vertexIndex0, vertexIndex1, vertexIndex2,
        normalIndex0, normalIndex1, normalIndex2
    );

    processTriangle(
        vertexIndex0, vertexIndex2, vertexIndex3,
        normalIndex0, normalIndex2, normalIndex3
    );

    return true;
}

void ObjParser::processFace(string &faceArgs)
{
    if (processDoubleFaceGeometryOnly(faceArgs)) { return; }
    if (processSingleFaceTriplets(faceArgs)) { return; }
    if (processSingleFaceTripletsVertexAndNormal(faceArgs)) { return; }
    if (processDoubleFaceTripletsVertexAndNormal(faceArgs)) { return; }

    string::size_type index;
    string rest = faceArgs;

    int index0 = std::stoi(rest, &index);

    rest = rest.substr(index);
    int index1 = std::stoi(rest, &index);

    rest = rest.substr(index);
    int index2 = std::stoi(rest, &index);

    // rest = rest.substr(index);
    // int index3 = std::stoi(rest, &index);
    // if (index3 < 3) {
    //     index3 += m_vertices.size();
    // } else {
    //     index3 -= 1;
    // }

    processTriangle(index0, index1, index2);
}

void ObjParser::processMaterialLibrary(std::string &libraryArgs)
{
    string filename = libraryArgs;
    MtlParser mtlParser(filename);
    mtlParser.parse();
    m_mtlLookup = mtlParser.materialLookup();
}

void ObjParser::processUseMaterial(std::string &materialArgs)
{
    string materialName = materialArgs;
    // std::cout << "Using material: " << materialName << std::endl;
    m_currentMaterialName = materialName;
}
