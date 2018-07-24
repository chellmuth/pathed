#include "obj_parser.h"

#include <iostream>

#include "triangle.h"


ObjParser::ObjParser(std::ifstream &objFile, Handedness handedness)
    : m_objFile(objFile), m_handedness(handedness)
{}

Scene ObjParser::parseScene()
{
    std::string line;
    while(std::getline(m_objFile, line)) {
        parseLine(line);
    }

    return Scene(
        m_faces,
        Point3(0.f, 0.f, -5.f)
    );
}

void ObjParser::parseLine(std::string &line)
{
    if (line.empty()) { return; }

    char command = line[0];
    if (command == '#') { return; }

    std::string rest = line.substr(2);

    if (command == 'v') {
        processVertex(rest);
    } else if (command == 'f') {
        processFace(rest);
    }
}

void ObjParser::processVertex(std::string &vertexArgs)
{
    std::string::size_type index = 0;
    std::string rest = vertexArgs;

    float x = std::stof(rest, &index);

    rest = rest.substr(index);
    float y = std::stof(rest, &index);

    rest = rest.substr(index);
    float z = std::stof(rest, &index);

    Point3 vertex(x, y, z);
    m_vertices.push_back(vertex);
}

void ObjParser::processFace(std::string &faceArgs)
{
    std::string::size_type index;
    std::string rest = faceArgs;

    int index0 = std::stoi(rest, &index);
    if (index0 < 0) {
        index0 += m_vertices.size();
    } else {
        index0 -= 1;
    }

    rest = rest.substr(index);
    int index1 = std::stoi(rest, &index);
    if (index1 < 0) {
        index1 += m_vertices.size();
    } else {
        index1 -= 1;
    }

    rest = rest.substr(index);
    int index2 = std::stoi(rest, &index);
    if (index2 < 2) {
        index2 += m_vertices.size();
    } else {
        index2 -= 1;
    }

    rest = rest.substr(index);
    int index3 = std::stoi(rest, &index);
    if (index3 < 3) {
        index3 += m_vertices.size();
    } else {
        index3 -= 1;
    }

    Triangle *face1, *face2;

    switch (m_handedness) {
    case Right:
        face1 = new Triangle(
            m_vertices[index0],
            m_vertices[index1],
            m_vertices[index2]
        );
        face2 = new Triangle(
            m_vertices[index2],
            m_vertices[index3],
            m_vertices[index0]
        );
        break;
    case Left:
        face1 = new Triangle(
            m_vertices[index1],
            m_vertices[index0],
            m_vertices[index2]
        );
        face2 = new Triangle(
            m_vertices[index3],
            m_vertices[index2],
            m_vertices[index0]
        );
        break;
    }

    m_faces.push_back(face1);
    m_faces.push_back(face2);
}
