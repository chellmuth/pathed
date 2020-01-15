#pragma once

#include "color.h"
#include "medium.h"
#include "point.h"
#include "random_generator.h"
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

    GridCell(const Point3 &point)
    {
        // Make sure the intersecting dimension stays "outside" the grid initially
        if (point.x() == 0.f) {
            x = -1;
        } else {
            x = (int)std::floor(point.x());
        }

        if (point.y() == 0.f) {
            y = -1;
        } else {
            y = (int)std::floor(point.y());
        }

        if (point.z() == 0.f) {
            z = -1;
        } else {
            z = (int)std::floor(point.z());
        }
    }

    bool operator==(const GridCell &cell) const {
        return x == cell.x && y == cell.y && z == cell.z;
    }

    bool operator!=(const GridCell &cell) const { return !(*this == cell); }
};

struct RegularTrackerStepResult {
    bool isValidStep;

    GridCell cell;
    float cellTime;
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

    Vector3 m_nextTimes;
};

class GridMedium : public Medium {
public:
    GridMedium(
        const GridInfo &gridInfo,
        const std::vector<float> &gridData,
        Color albedo,
        float scale
    );

    GridMedium(
        const GridInfo &gridInfo,
        const std::vector<float> &gridData
    ) : GridMedium(gridInfo, gridData, Color(0.f), 1.f) {}

    Color transmittance(const Point3 &pointA, const Point3 &pointB) const override;

    TransmittanceQueryResult findTransmittance(
        const Point3 &entryPointWorld,
        const Point3 &exitPointWorld,
        float targetTransmittance
    ) const override;

    float sigmaT(const Point3 &worldPoint) const override;
    float sigmaS(const Point3 &worldPoint) const override;

    Color integrate(
        const Point3 &entryPointWorld,
        const Point3 &exitPointWorld,
        const Scene &scene,
        RandomGenerator &random
    ) const override;

protected:
    Color directSampleLights(
        const Point3 &point,
        const Scene &scene,
        RandomGenerator &random
    ) const;

    Point3 worldToGrid(const Point3 &worldPoint) const;
    float lookup(int cellX, int cellY, int cellZ) const;


    GridInfo m_gridInfo;

    float m_widthX;
    float m_widthY;
    float m_widthZ;

    std::vector<float> m_gridData;

    Color m_albedo;
    float m_scale;
};
