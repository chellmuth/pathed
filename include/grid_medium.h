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

struct RegularTrackerStepResult {
    bool validStep;

    GridCell cell;
    /* Point3 entryPoint; */
    /* Point3 exitPoint; */
};

class RegularTrackerState {
public:
    RegularTrackerState(const GridInfo &gridInfo, const Point3& entryPoint, const Point3 &exitPoint);

    RegularTrackerStepResult step();

private:
    const GridInfo m_gridInfo;
    const Point3 m_entryPoint;
    const Point3 m_exitPoint;

    Vector3 m_rates;

    float m_currentTime;
    GridCell m_currentCell;

    Vector3 m_nextTimes;
};

class GridMedium : public Medium {
public:
    GridMedium(const GridInfo &gridInfo, const std::vector<float> &gridData);

    Color transmittance(const Point3 &pointA, const Point3 &pointB) const override;
    float findTransmittance(const Point3 &entryPointWorld, const Point3 &exitPointWorld, float targetTransmission) const;

protected:
    Point3 worldToGrid(const Point3 &worldPoint) const;
    Point3 gridToWorld(const Point3 &gridPoint) const;
    float lookup(int cellX, int cellY, int cellZ) const;


    GridInfo m_gridInfo;

    float m_widthX;
    float m_widthY;
    float m_widthZ;

    std::vector<float> m_gridData;
};
