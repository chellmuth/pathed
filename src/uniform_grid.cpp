#include "uniform_grid.h"

UniformGrid::UniformGrid(
    const GridInfo &gridInfo,
    const std::vector<float> &gridData
) : m_gridInfo(gridInfo),
    m_gridData(gridData)
{}

float UniformGrid::lookup(int cellX, int cellY, int cellZ) const
{
    if (cellX < 0 || cellX >= m_gridInfo.cellsX) { return 0.f; }
    if (cellY < 0 || cellY >= m_gridInfo.cellsY) { return 0.f; }
    if (cellZ < 0 || cellZ >= m_gridInfo.cellsZ) { return 0.f; }

    const int index = (cellZ * m_gridInfo.cellsY + cellY) * m_gridInfo.cellsX + cellX;
    const float density = m_gridData[index];

    return density;
}

float UniformGrid::interpolate(Point3 gridPoint) const
{
    return interpolate(gridPoint.x(), gridPoint.y(), gridPoint.z());
}

float UniformGrid::interpolate(float gridX, float gridY, float gridZ) const
{
    const int x0 = std::floor(gridX);
    const int x1 = x0 + 1;
    const float xd = (gridX - x0) / (x1 - x0);

    const int y0 = std::floor(gridY);
    const int y1 = y0 + 1;
    const float yd = (gridY - y0) / (y1 - y0);

    const int z0 = std::floor(gridZ);
    const int z1 = z0 + 1;
    const float zd = (gridZ - z0) / (z1 - z0);

    const float c000 = lookup(x0, y0, z0);
    const float c001 = lookup(x0, y0, z1);
    const float c010 = lookup(x0, y1, z0);
    const float c011 = lookup(x0, y1, z1);
    const float c100 = lookup(x1, y0, z0);
    const float c101 = lookup(x1, y0, z1);
    const float c110 = lookup(x1, y1, z0);
    const float c111 = lookup(x1, y1, z1);

    const float c_00 = c000 * (1 - xd) + c100 * xd;
    const float c_01 = c001 * (1 - xd) + c101 * xd;
    const float c_10 = c010 * (1 - xd) + c110 * xd;
    const float c_11 = c011 * (1 - xd) + c111 * xd;

    const float c__0 = c_00 * (1 - yd) + c_10 * yd;
    const float c__1 = c_01 * (1 - yd) + c_11 * yd;

    const float c = c__0 * (1 - zd) + c__1 * zd;
    return c;
}
