#pragma once

#include "color.h"
#include "medium.h"
#include "point.h"
#include "random_generator.h"
#include "vector.h"

#include <cmath>
#include <ostream>
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

    float m_endTime;

    Vector3 m_nextTimes;
};

class GridMedium : public Medium {
public:
    GridMedium(
        const GridInfo &gridInfo,
        const std::vector<float> &gridData,
        float albedo,
        float scale
    );

    GridMedium(
        const GridInfo &gridInfo,
        const std::vector<float> &gridData
    ) : GridMedium(gridInfo, gridData, 0.f, 1.f) {}

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
    Point3 worldToGrid(const Point3 &worldPoint) const;
    float lookupSigmaT(int cellX, int cellY, int cellZ) const;


    GridInfo m_gridInfo;

    float m_widthX;
    float m_widthY;
    float m_widthZ;

    std::vector<float> m_gridData;

    float m_albedo;
    float m_scale;
};
