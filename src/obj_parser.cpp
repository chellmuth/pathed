#include "obj_parser.h"

#include <assert.h>
#include <iostream>

#include "color.h"
#include "string_util.h"
#include "triangle.h"

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

    return Scene(
        m_surfaces,
        Point3(0.f, 1.9f, 0.f)
    );
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

void ObjParser::processFace(string &faceArgs)
{
    string::size_type index;

    std::queue<string> tokens = tokenize(faceArgs);
    assert(tokens.size() == 3 || tokens.size() == 4);

    int index0 = std::stoi(tokens.front());
    if (index0 < 0) {
        index0 += m_vertices.size();
    } else {
        index0 -= 1;
    }
    tokens.pop();

    int index1 = std::stoi(tokens.front());
    if (index1 < 0) {
        index1 += m_vertices.size();
    } else {
        index1 -= 1;
    }
    tokens.pop();

    int index2 = std::stoi(tokens.front());
    if (index2 < 2) {
        index2 += m_vertices.size();
    } else {
        index2 -= 1;
    }
    tokens.pop();

    Color diffuse = m_materialLookup[m_currentMaterialName].diffuse;
    std::shared_ptr<Material> material(new Material(diffuse));

    Triangle *face1;
    switch (m_handedness) {
    case Right:
        face1 = new Triangle(
            m_vertices[index0],
            m_vertices[index1],
            m_vertices[index2]
        );
        break;
    case Left:
        face1 = new Triangle(
            m_vertices[index1],
            m_vertices[index0],
            m_vertices[index2]
        );
        break;
    }

    std::shared_ptr<Triangle> shape1(face1);

    m_surfaces.push_back(
        std::shared_ptr<Surface>(new Surface(shape1, material))
    );

    if (tokens.empty()) { return; }

    int index3 = std::stoi(tokens.front());
    if (index3 < 3) {
        index3 += m_vertices.size();
    } else {
        index3 -= 1;
    }
    tokens.pop();

    Triangle *face2;

    switch (m_handedness) {
    case Right:
        face2 = new Triangle(
            m_vertices[index2],
            m_vertices[index3],
            m_vertices[index0]
        );
        break;
    case Left:
        face2 = new Triangle(
            m_vertices[index3],
            m_vertices[index2],
            m_vertices[index0]
        );
        break;
    }

    std::shared_ptr<Triangle> shape2(face2);
    m_surfaces.push_back(
        std::shared_ptr<Surface>(new Surface(shape2, material))
    );
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
