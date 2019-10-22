#include "obj_parser.h"

#include "camera.h"
#include "color.h"
#include "globals.h"
#include "lambertian.h"
#include "primitive.h"
#include "string_util.h"
#include "triangle.h"
#include "vector.h"

#include <iostream>
#include <memory>
#include <regex>
#include <vector>

using string = std::string;

ObjParser::ObjParser(
    std::ifstream &objFile,
    const Transform &transform,
    bool useFaceNormals,
    Handedness handedness
)
    : m_objFile(objFile),
      m_transform(transform),
      m_useFaceNormals(useFaceNormals),
      m_handedness(handedness),
      m_currentGroup("")
{
    m_geometry = rtcNewGeometry(g_rtcDevice, RTC_GEOMETRY_TYPE_TRIANGLE);
}

std::vector<std::shared_ptr<Surface> > ObjParser::parse()
{
    string line;
    while(std::getline(m_objFile, line)) {
        parseLine(line);
    }

    RTCGeometry rtcMesh = rtcNewGeometry(g_rtcDevice, RTC_GEOMETRY_TYPE_TRIANGLE);
    float *rtcVertices = (float *)rtcSetNewGeometryBuffer(
        rtcMesh,                /* geometry */
        RTC_BUFFER_TYPE_VERTEX, /* type */
        0,                      /* slot */
        RTC_FORMAT_FLOAT3,      /* format */
        3 * sizeof(float),      /* byte stride */
        m_vertices.size()       /* item count */
    );

    int i = 0;
    for (auto &vertex : m_vertices) {
        rtcVertices[i * 3 + 0] = vertex.x();
        rtcVertices[i * 3 + 1] = vertex.y();
        rtcVertices[i * 3 + 2] = vertex.z();

        i += 1;
    }

    unsigned int *rtcFaces = (unsigned int *)rtcSetNewGeometryBuffer(
        rtcMesh,
        RTC_BUFFER_TYPE_INDEX,
        0,
        RTC_FORMAT_UINT3,
        3 * sizeof(unsigned int),
        m_faces.size() / 3
    );

    i = 0;
    for (auto face : m_faces) {
        rtcFaces[i] = face;
        i += 1;
    }

    rtcSetGeometryVertexAttributeCount(rtcMesh, 1);
    float *rtcUVs = (float *)rtcSetNewGeometryBuffer(
        rtcMesh,                          /* geometry */
        RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, /* type */
        0,                                /* slot */
        RTC_FORMAT_FLOAT2,                /* format */
        2 * sizeof(float),                /* byte stride */
        m_vertices.size()                 /* item count */
    );

    const unsigned long vertexUVsSize = m_vertexUVs.size();
    const unsigned long verticesSize = m_vertices.size();
    if (vertexUVsSize < verticesSize) {
        std::cout << "Reserving more vertex UVs (" << vertexUVsSize << ", " << verticesSize << ")" << std::endl;
        m_vertexUVs.resize(m_vertices.size());
        for (int i = vertexUVsSize; i < verticesSize; i++) {
            m_vertexUVs[i] = {0.f, 0.f};
        }
    }

    for (i = 0; i < m_vertices.size(); i++) {
        rtcUVs[2 * i + 0] = m_vertexUVs[i].u;
        rtcUVs[2 * i + 1] = m_vertexUVs[i].v;
    }

    // i = 0;
    // for (auto &uv : m_uvs) {
    //     rtcUVs[2 * i + 0] = uv.u;
    //     rtcUVs[2 * i + 1] = uv.v;
    //     i += 1;
    // }

    // // If this object doesn't do UVs, supply them anyway for now
    // if (i == 0) {
    //     for (i = 0; i < m_vertices.size() * 2; i++) {
    //         rtcUVs[i] = 0.f;
    //     }
    // }

    rtcCommitGeometry(rtcMesh);

    unsigned int rtcGeometryID = rtcAttachGeometry(g_rtcScene, rtcMesh);
    rtcReleaseGeometry(rtcMesh);

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

    Point3 vertex(x, y, z);
    m_vertices.push_back(m_transform.apply(vertex));
}

void ObjParser::processNormal(string &normalArgs)
{
    string::size_type index = 0;
    string rest = normalArgs;

    float x = std::stof(rest, &index);

    rest = rest.substr(index);
    float y = std::stof(rest, &index);

    rest = rest.substr(index);
    float z = std::stof(rest, &index);

    Vector3 normal(x, y, z);
    m_normals.push_back(normal);
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
    Color diffuse = m_materialLookup[m_currentMaterialName].diffuse;
    Color emit = m_materialLookup[m_currentMaterialName].emit;
    auto material = std::make_shared<Lambertian>(diffuse, emit);

    std::shared_ptr<Triangle> shape(face);
    std::shared_ptr<Surface> surface(new Surface(shape, material));

    m_surfaces.push_back(surface);

    if (emit.isBlack()) { return; }

    std::shared_ptr<Light> light(new Light(surface));

    m_lights.push_back(light);
}

void ObjParser::processTriangle(
    int vertexIndex0, int vertexIndex1, int vertexIndex2,
    int UVIndex0, int UVIndex1, int UVIndex2
) {

    correctIndices(m_vertices, &vertexIndex0, &vertexIndex1, &vertexIndex2);
    correctIndices(m_vertices, &UVIndex0, &UVIndex1, &UVIndex2);

    Triangle *face = new Triangle(
        m_vertices[vertexIndex0],
        m_vertices[vertexIndex1],
        m_vertices[vertexIndex2],
        m_uvs[UVIndex0],
        m_uvs[UVIndex1],
        m_uvs[UVIndex2]
    );

    processFace(face);

    m_faces.push_back(vertexIndex0);
    m_faces.push_back(vertexIndex1);
    m_faces.push_back(vertexIndex2);

    m_vertexUVs.resize(m_vertices.size());
    m_vertexUVs[vertexIndex0] = m_uvs[UVIndex0];
    m_vertexUVs[vertexIndex1] = m_uvs[UVIndex1];
    m_vertexUVs[vertexIndex2] = m_uvs[UVIndex2];
}

void ObjParser::processTriangle(int vertexIndex0, int vertexIndex1, int vertexIndex2)
{
    correctIndices(m_vertices, &vertexIndex0, &vertexIndex1, &vertexIndex2);

    Triangle *face = new Triangle(
        m_vertices[vertexIndex0],
        m_vertices[vertexIndex1],
        m_vertices[vertexIndex2]
    );

    processFace(face);

    m_faces.push_back(vertexIndex0);
    m_faces.push_back(vertexIndex1);
    m_faces.push_back(vertexIndex2);
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
    static std::regex expression("(-?\\d+)/(-?\\d*)/(-?\\d+) (-?\\d+)/(-?\\d*)/(-?\\d+) (-?\\d+)/(-\\d*)/(-?\\d+)\\s*");
    std::smatch match;
    std::regex_match (faceArgs, match, expression);

    if (match.empty()) {
        return false;
    }

    int vertexIndex0 = std::stoi(match[1]);
    int vertexIndex1 = std::stoi(match[4]);
    int vertexIndex2 = std::stoi(match[7]);

    int UVIndex0 = std::stoi(match[3]);
    int UVIndex1 = std::stoi(match[6]);
    int UVIndex2 = std::stoi(match[9]);

    processTriangle(
        vertexIndex0, vertexIndex1, vertexIndex2,
        UVIndex0, UVIndex1, UVIndex2
    );

    return true;
}

void ObjParser::processFace(string &faceArgs)
{
    if (processDoubleFaceGeometryOnly(faceArgs)) { return; }
    if (processSingleFaceTriplets(faceArgs)) { return; }

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
    m_materialLookup = mtlParser.materialLookup();
}

void ObjParser::processUseMaterial(std::string &materialArgs)
{
    string materialName = materialArgs;
    MtlMaterial currentMaterial = m_materialLookup[materialName];
    Color color = currentMaterial.diffuse;

    std::cout << "Using material: " << materialName << " | Diffuse: " << color.r() << " " << color.g() << " " << color.b() <<std::endl;

    m_currentMaterialName = materialArgs;
}
