#include "grid_medium.h"

GridMedium::GridMedium(const GridInfo &gridInfo, const std::vector<float> &gridData)
    : m_gridInfo(gridInfo), m_gridData(gridData)
{
}

float GridMedium::lookup(int cellX, int cellY, int cellZ)
{
    return m_gridData[(cellZ * m_gridInfo.cellsY + cellY) * m_gridInfo.cellsX + cellX];
}

