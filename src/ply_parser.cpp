#include "ply_parser.h"

#include <assert.h>
#include <iostream>
#include <regex>
#include <string>

using string = std::string;

PLYParser::PLYParser(
    std::ifstream &objFile,
    const Transform &transform,
    bool useFaceNormals,
    Handedness handedness
) : m_objFile(objFile),
    m_transform(transform),
    m_useFaceNormals(useFaceNormals),
    m_handedness(handedness)
{}

std::vector<std::shared_ptr<Surface> > PLYParser::parse()
{
    string header;
    std::getline(m_objFile, header);
    assert(header == "ply");

    string format;
    std::getline(m_objFile, format);
    assert(format == "format binary_little_endian 1.0");

    string vertexElement;
    std::getline(m_objFile, vertexElement);

    static std::regex vertexElementExpression("element vertex (\\d+)");
    std::smatch vertexElementMatch;
    std::regex_match(vertexElement, vertexElementMatch, vertexElementExpression);
    assert(!vertexElementMatch.empty());

    const int vertexCount = std::stoi(vertexElementMatch[1]);
    std::cout << vertexCount << std::endl;

    string propertyX;
    string propertyY;
    string propertyZ;

    std::getline(m_objFile, propertyX);
    std::getline(m_objFile, propertyY);
    std::getline(m_objFile, propertyZ);

    assert(propertyX == "property float x");
    assert(propertyY == "property float y");
    assert(propertyZ == "property float z");

    string faceElement;
    std::getline(m_objFile, faceElement);

    static std::regex faceElementExpression("element face (\\d+)");
    std::smatch faceElementMatch;
    std::regex_match(faceElement, faceElementMatch, faceElementExpression);
    assert(!faceElementMatch.empty());

    const int faceCount = std::stoi(faceElementMatch[1]);
    std::cout << faceCount << std::endl;

    string propertyVertexIndices;
    std::getline(m_objFile, propertyVertexIndices);
    assert(propertyVertexIndices == "property list uint8 int vertex_indices");

    string endHeader;
    std::getline(m_objFile, endHeader);
    assert(endHeader == "end_header");

    for (int i = 0; i < vertexCount; i++) {
        float point[3];
        m_objFile.read((char *)&point, 4 * 3);
    }

    for (int i = 0; i < faceCount; i++) {
        uint8_t faceSize;
        m_objFile.read((char *)&faceSize, 1);

        std::cout << (int)faceSize << std::endl;

        int index[faceSize];
        m_objFile.read((char *)&index, faceSize * 3);
    }

    assert(m_objFile.eof());

    std::vector<std::shared_ptr<Surface> > result;
    return result;
}
