#pragma once

#include "color.h"
#include "medium.h"

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

class GridMedium : public Medium {
public:
    GridMedium(const GridInfo &gridInfo, const std::vector<float> &gridData);
    float lookup(int x, int y, int z);

    Color sigmaT() const override { return Color(0.f); }

protected:
    GridInfo m_gridInfo;
    std::vector<float> m_gridData;
};
