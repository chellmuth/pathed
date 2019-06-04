#include "obj_parser.h"

#include "camera.h"
#include "color.h"
#include "lambertian.h"
#include "string_util.h"
#include "triangle.h"

#include <iostream>
#include <regex>

using string = std::string;

ObjParser::ObjParser(std::ifstream &objFile, Handedness handedness)
    : m_objFile(objFile), m_handedness(handedness), m_currentGroup("")
{}

Scene ObjParser::parseScene()
{
    string line;
    while(std::getline(m_objFile, line)) {
        parseLine(line);
    }

    std::shared_ptr<Camera> camera;
    return Scene(m_surfaces, m_lights, camera);
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

    rest = rest.substr(index);
    float y = std::stof(rest, &index);

    rest = rest.substr(index);
    float z = std::stof(rest, &index);

    Point3 vertex(x, y, z);
    m_vertices.push_back(vertex);
}

void ObjParser::processGroup(string &groupArgs)
{
    string name = lTrim(groupArgs);
    m_currentGroup = name;
}

void ObjParser::processTriangle(int index0, int index1, int index2)
{
    if (index0 < 0) {
        index0 = m_vertices.size() + index0;
    } else {
        index0 -= 1;
    }

    if (index1 < 0) {
        index1 = m_vertices.size() + index1;
    } else {
        index1 -= 1;
    }

    if (index2 < 0) {
        index2 = m_vertices.size() + index2;
    } else {
        index2 -= 1;
    }

    Triangle *face;
    switch (m_handedness) {
    case Right:
        face = new Triangle(
            m_vertices[index0],
            m_vertices[index1],
            m_vertices[index2]
        );
        break;
    case Left:
        face = new Triangle(
            m_vertices[index1],
            m_vertices[index0],
            m_vertices[index2]
        );
        break;
    }

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
    static std::regex expression("(\\d+)/(\\d*)/(\\d+) (\\d+)/(\\d*)/(\\d+) (\\d+)/(\\d*)/(\\d+)\\s*");
    std::smatch match;
    std::regex_match (faceArgs, match, expression);

    if (match.empty()) {
        return false;
    }

    int index0 = std::stoi(match[1]);
    int index1 = std::stoi(match[4]);
    int index2 = std::stoi(match[7]);

    processTriangle(index0, index1, index2);

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
