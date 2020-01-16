#pragma once

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

    float widthX() const { return maxX - minX; }
    float widthY() const { return maxY - minY; }
    float widthZ() const { return maxZ - minZ; }
};

struct GridCell {
    int x;
    int y;
    int z;

    GridCell(int x_, int y_, int z_) : x(x_), y(y_), z(z_) {}

    GridCell(const Point3 &point, const GridInfo &gridInfo)
    {
        // Bring outer boundaries "into" the cell that will be integrated over
        if (point.x() == (float)gridInfo.cellsX) {
            x = (float)gridInfo.cellsX - 1;
        } else {
            x = (int)std::floor(point.x());
        }

        if (point.y() == (float)gridInfo.cellsY) {
            y = (float)gridInfo.cellsY - 1;
        } else {
            y = (int)std::floor(point.y());
        }

        if (point.z() == (float)gridInfo.cellsZ) {
            z = (float)gridInfo.cellsZ - 1;
        } else {
            z = (int)std::floor(point.z());
        }
    }

    bool operator==(const GridCell &cell) const {
        return x == cell.x && y == cell.y && z == cell.z;
    }

    bool operator!=(const GridCell &cell) const { return !(*this == cell); }
};

std::ostream &operator<<(std::ostream &os, const GridCell &gridCell);

class UniformGrid {
public:
    UniformGrid(
        const GridInfo &gridInfo,
        const std::vector<float> &gridData
    );

    float lookup(int cellX, int cellY, int cellZ) const;

    float interpolate(Point3 gridPoint) const;
    float interpolate(float gridX, float gridY, float gridZ) const;

private:
    GridInfo m_gridInfo;

    float m_widthX;
    float m_widthY;
    float m_widthZ;

    std::vector<float> m_gridData;
};
