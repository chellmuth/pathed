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

    std::vector<float> m_gridData;
};
