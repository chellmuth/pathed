#pragma once

#include "uniform_grid.h"
#include "point.h"
#include "vector.h"

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
