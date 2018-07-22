#include "obj_parser.h"

#include <iostream>

#include "triangle.h"

Scene ObjParser::parseScene(std::ifstream &objFile)
{
    std::string line;
    while(std::getline(objFile, line)) {
        parseLine(line);
    }

    return Scene(
        m_faces,
        Point3(0.f, 0.f, 0.f)
    );
}

void ObjParser::parseLine(std::string &line)
{
    if (line.empty()) { return; }

    char command = line[0];
    std::string rest = line.substr(2);

    if (command == 'v') {
        processVertex(rest);
    } else if (command == 'f') {
        processFace(rest);
    }
}

void ObjParser::processVertex(std::string &vertexArgs)
{
    std::string::size_type index;
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

    rest = rest.substr(index);
    int index1 = std::stoi(rest, &index);

    rest = rest.substr(index);
    int index2 = std::stoi(rest, &index);

    rest = rest.substr(index);
    int index3 = std::stoi(rest, &index);

    Triangle *face1 = new Triangle(
        m_vertices[index0 - 1],
        m_vertices[index1 - 1],
        m_vertices[index2 - 1]
    );

    m_faces.push_back(face1);

    Triangle *face2 = new Triangle(
        m_vertices[index2 - 1],
        m_vertices[index3 - 1],
        m_vertices[index0 - 1]
    );

    m_faces.push_back(face2);

}
