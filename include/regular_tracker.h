#pragma once

#include "uniform_grid.h"
#include "point.h"
#include "vector.h"

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

struct RegularTrackerStepResult {
    bool isValidStep;

    GridCell cell;
    float cellTime;
    float enterTime;
    float currentTime;
};

class RegularTrackerState {
public:
    RegularTrackerState(const GridInfo &gridInfo, const Point3& entryPoint, const Point3 &exitPoint);

    RegularTrackerStepResult step();

private:
    float worldTime(float gridTime);

    const GridInfo m_gridInfo;
    const Point3 m_entryPoint;
    const Point3 m_exitPoint;

    Vector3 m_rates;

    float m_currentTime;
    GridCell m_currentCell;

    float m_endTime;

    Vector3 m_nextTimes;
};
