#include "uniform_grid.h"

#include <iostream>

static bool DEBUG = false;

UniformGrid::UniformGrid(
    int cellsX, int cellsY, int cellsZ,
    const std::vector<float> &gridData
) : m_cellsX(cellsX),
    m_cellsY(cellsY),
    m_cellsZ(cellsZ),
    m_gridData(gridData)
{}

float UniformGrid::lookup(int cellX, int cellY, int cellZ) const
{
    if (cellX < 0 || cellX >= m_cellsX) { return 0.f; }
    if (cellY < 0 || cellY >= m_cellsY) { return 0.f; }
    if (cellZ < 0 || cellZ >= m_cellsZ) { return 0.f; }

    const int index = (cellZ * m_cellsY + cellY) * m_cellsX + cellX;
    const float density = m_gridData[index];

    return density;
}

float UniformGrid::interpolate(Point3 gridPoint) const
{
    return interpolate(gridPoint.x(), gridPoint.y(), gridPoint.z());
}

float UniformGrid::interpolate(float gridX, float gridY, float gridZ) const
{
    if (gridX < 0 || gridX > m_cellsX - 1) { return 0.f; }
    if (gridY < 0 || gridY > m_cellsY - 1) { return 0.f; }
    if (gridZ < 0 || gridZ > m_cellsZ - 1) { return 0.f; }

    const int x0 = std::floor(gridX);
    const int x1 = std::ceil(gridX);
    const float xd = gridX - x0;

    const int y0 = std::floor(gridY);
    const int y1 = std::ceil(gridY);
    const float yd = gridY - y0;

    const int z0 = std::floor(gridZ);
    const int z1 = std::ceil(gridZ);
    const float zd = gridZ - z0;

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

    if (DEBUG) {
        std::cout << "Interpolation Debug: "
            << "(" << gridX << ", " << gridY << ", " << gridZ << ")" << std::endl;
        std::cout << "x0: " << x0 << " x1: " << x1 << " xd: " << xd << std::endl;
        std::cout << "y0: " << y0 << " y1: " << y1 << " yd: " << yd << std::endl;
        std::cout << "z0: " << z0 << " z1: " << z1 << " zd: " << zd << std::endl;
    }

    return c;
}
