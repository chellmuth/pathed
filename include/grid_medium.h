#pragma once

#include "color.h"
#include "medium.h"
#include "point.h"
#include "vector.h"

#include <cmath>
#include <vector>

struct GridInfo {
    uint32_t cellsX;
    uint32_t cellsY;
    uint32_t cellsZ;

    float minX;
    float minY;
    float minZ;

    float maxX;
    float maxY;
    float maxZ;
};

struct GridCell {
    int x;
    int y;
    int z;

    GridCell(const Point3 &point)
        : x(std::floor(point.x())),
          y(std::floor(point.y())),
          z(std::floor(point.z()))
    {}

    bool operator==(const GridCell &cell) const {
        return x == cell.x && y == cell.y && z == cell.z;
    }

    bool operator!=(const GridCell &cell) const { return !(*this == cell); }
};

class GridMedium : public Medium {
public:
    GridMedium(const GridInfo &gridInfo, const std::vector<float> &gridData);

    Color transmittance(const Point3 &pointA, const Point3 &pointB) const override;
    float lookup(int x, int y, int z) const;

protected:
    Point3 worldToGrid(const Point3 &worldPoint) const;
    Point3 gridToWorld(const Point3 &gridPoint) const;
    bool validCell(const GridCell &cell) const;


    GridInfo m_gridInfo;

    float m_widthX;
    float m_widthY;
    float m_widthZ;

    std::vector<float> m_gridData;
};
