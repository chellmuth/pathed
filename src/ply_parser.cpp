#include "ply_parser.h"

#include "color.h"
#include "geometry_parser.h"
#include "globals.h"
#include "lambertian.h"
#include "point.h"
#include "triangle.h"
#include "uv.h"
#include "vector.h"

#include <assert.h>
#include <iostream>
#include <memory>
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
    std::vector<std::shared_ptr<Surface> > surfaces;

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

    std::vector<Point3> vertices;
    for (int i = 0; i < vertexCount; i++) {
        float point[3];
        m_objFile.read((char *)&point, 4 * 3);

        Point3 vertex(point[0], point[1], point[2]);
        vertices.push_back(m_transform.apply(vertex));
    }

    std::vector<FaceIndices> faceIndices;
    for (int i = 0; i < faceCount; i++) {
        uint8_t faceSize;
        m_objFile.read((char *)&faceSize, 1);

        assert(faceSize == 3);

        int index[faceSize];
        m_objFile.read((char *)&index, faceSize * 4);

        for (int j = 0; j < faceSize; j ++) {
            assert(index[j] < vertexCount);
        }

        faceIndices.push_back({
            .vertices = {
                { index[0], -1, -1 },
                { index[1], -1, -1 },
                { index[2], -1, -1 }
            }
        });

        const Color diffuse(0.f, 1.f, 0.f);
        const Color emit(0.f, 0.f, 0.f);
        auto material = std::make_shared<Lambertian>(diffuse, emit);

        auto shape = std::make_shared<Triangle>(
            vertices[index[0]],
            vertices[index[1]],
            vertices[index[2]]
        );
        auto surface = std::make_shared<Surface>(
            shape,
            material,
            nullptr
        );

        surfaces.push_back(surface);
    }

    string rest;
    if (std::getline(m_objFile, rest)) {
        assert(false);
    }

    assert(m_objFile.eof());

    std::vector<UV> vertexUVs;
    std::vector<Vector3> vertexNormals;
    GeometryParser::processRTCGeometry(
        g_rtcScene,
        vertices,
        vertexUVs,
        vertexNormals,
        faceIndices
    );

    return surfaces;
}
