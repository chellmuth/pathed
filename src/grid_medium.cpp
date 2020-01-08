#include "grid_medium.h"

#include "util.h"

#include <assert.h>

GridMedium::GridMedium(const GridInfo &gridInfo, const std::vector<float> &gridData)
    : m_gridInfo(gridInfo), m_gridData(gridData)
{
    m_widthX = m_gridInfo.maxX - m_gridInfo.minX;
    m_widthY = m_gridInfo.maxY - m_gridInfo.minY;
    m_widthZ = m_gridInfo.maxZ - m_gridInfo.minZ;
}

float GridMedium::lookup(int cellX, int cellY, int cellZ) const
{
    return m_gridData[(cellZ * m_gridInfo.cellsY + cellY) * m_gridInfo.cellsX + cellX];
}

static float calculateNextDistance(float currentValue, bool isForward)
{
    if (isForward) {
        if (currentValue == std::floor(currentValue)) {
            return currentValue + 1.f;
        } else {
            return std::ceil(currentValue) - currentValue;
        }
    } else {
        if (currentValue == std::floor(currentValue)) {
            return currentValue - 1.f;
        } else {
            return std::floor(currentValue) - currentValue;
        }
    }
}

static Vector3 calculateNextTimes(const Vector3 &rates, const Point3 &entryPoint)
{
    const Vector3 nextDistances(
        calculateNextDistance(entryPoint.x(), rates.x()),
        calculateNextDistance(entryPoint.y(), rates.y()),
        calculateNextDistance(entryPoint.z(), rates.z())
    );

    return nextDistances / rates;
}

static GridCell updateCell(const GridCell &cell, const std::string &attribute, int amount)
{
    GridCell result = cell;
    if (attribute == "x") {
        result.x += amount;
    } else if (attribute == "y") {
        result.y += amount;
    } else if (attribute == "z") {
        result.z += amount;
    }

    return result;
}

static Vector3 updateTimes(const Vector3 &nextTimes, const std::string &attribute, float amount)
{
    if (attribute == "x") {
        return Vector3(
            nextTimes.x() + amount,
            nextTimes.y(),
            nextTimes.z()
        );
    }

    if (attribute == "y") {
        return Vector3(
            nextTimes.x(),
            nextTimes.y() + amount,
            nextTimes.z()
        );
    }

    if (attribute == "z") {
        return Vector3(
            nextTimes.x(),
            nextTimes.y(),
            nextTimes.z() + amount
        );
    }

    assert(0);
}

Point3 GridMedium::worldToGrid(const Point3 &worldPoint) const
{
    return Point3(
        ((worldPoint.x() - m_gridInfo.minX) / m_widthX) * m_gridInfo.cellsX,
        ((worldPoint.y() - m_gridInfo.minY) / m_widthY) * m_gridInfo.cellsY,
        ((worldPoint.z() - m_gridInfo.minZ) / m_widthZ) * m_gridInfo.cellsZ
    );
}

Point3 GridMedium::gridToWorld(const Point3 &gridPoint) const
{
    return Point3(
        (gridPoint.x() / m_gridInfo.cellsX) * m_widthX + m_gridInfo.minX,
        (gridPoint.y() / m_gridInfo.cellsY) * m_widthY + m_gridInfo.minY,
        (gridPoint.z() / m_gridInfo.cellsZ) * m_widthZ + m_gridInfo.minZ
    );
}

bool GridMedium::validCell(const GridCell &cell) const
{
    if (cell.x < 0 || cell.y < 0 || cell.z < 0) { return false; }
    if (cell.x >= m_gridInfo.cellsX) { return false; }
    if (cell.y >= m_gridInfo.cellsY) { return false; }
    if (cell.z >= m_gridInfo.cellsZ) { return false; }

    return true;
}

Color GridMedium::transmittance(const Point3 &entryPointWorld, const Point3 &exitPointWorld) const
{
    const Point3 entryPoint = worldToGrid(entryPointWorld);
    const Point3 exitPoint = worldToGrid(exitPointWorld);

    const Vector3 rayPath = (exitPoint - entryPoint).toVector();
    const float totalDistance = rayPath.length();

    const Vector3 spans = (exitPoint - entryPoint).toVector();
    const Vector3 rates = spans / totalDistance;
    Vector3 nextTimes = calculateNextTimes(rates, entryPoint);

    float currentTime = 0.f;
    float accumulatedExponent = 0.f;

    GridCell currentCell(entryPoint);
    while(validCell(currentCell)) {
        const float minTime = nextTimes.min();

        const float cellTime = minTime - currentTime;
        const float sigmaT = lookup(currentCell.x, currentCell.y, currentCell.z);

        accumulatedExponent += -sigmaT * cellTime;
        currentTime = minTime;

        if (nextTimes.x() == minTime) {
            if (rates.x() > 0.f) {
                nextTimes = updateTimes(nextTimes, "x", 1.f / rates.x());
                currentCell = updateCell(currentCell, "x", 1);
            } else {
                nextTimes = updateTimes(nextTimes, "x", -1.f / rates.x());
                currentCell = updateCell(currentCell, "x", -1);
            }
        } else if (nextTimes.y() == minTime) {
            if (rates.y() > 0.f) {
                nextTimes = updateTimes(nextTimes, "y", 1.f / rates.y());
                currentCell = updateCell(currentCell, "y", 1);
            } else {
                nextTimes = updateTimes(nextTimes, "y", -1.f / rates.y());
                currentCell = updateCell(currentCell, "y", -1);
            }
        } else if (nextTimes.z() == minTime) {
            if (rates.z() > 0.f) {
                nextTimes = updateTimes(nextTimes, "z", 1.f / rates.z());
                currentCell = updateCell(currentCell, "z", 1);
            } else {
                nextTimes = updateTimes(nextTimes, "z", -1.f / rates.z());
                currentCell = updateCell(currentCell, "z", -1);
            }
        } else { assert(0); }
    }

    return util::exp(accumulatedExponent);
}

float GridMedium::findTransmittance(const Point3 &entryPointWorld, const Point3 &exitPointWorld, float targetTransmission) const
{
    const float targetExponent = std::log(targetTransmission);

    const Point3 entryPoint = worldToGrid(entryPointWorld);
    const Point3 exitPoint = worldToGrid(exitPointWorld);

    const Vector3 rayPath = (exitPoint - entryPoint).toVector();
    const float totalDistance = rayPath.length();

    const Vector3 spans = (exitPoint - entryPoint).toVector();
    const Vector3 rates = spans / totalDistance;
    Vector3 nextTimes = calculateNextTimes(rates, entryPoint);

    float currentTime = 0.f;
    float accumulatedExponent = 0.f;

    GridCell currentCell(entryPoint);
    while(validCell(currentCell)) {
        const float minTime = nextTimes.min();

        const float cellTime = minTime - currentTime;
        const float sigmaT = lookup(currentCell.x, currentCell.y, currentCell.z);

        const float cellExponent = -sigmaT * cellTime;
        accumulatedExponent += cellExponent;
        if (accumulatedExponent > targetTransmission) {
            const float overflow = accumulatedExponent - targetTransmission;
            const float cellRatio = overflow / cellExponent;
            const float actualCellTime = cellTime * cellRatio;

            return currentTime + actualCellTime;
        }

        currentTime = minTime;

        if (nextTimes.x() == minTime) {
            if (rates.x() > 0.f) {
                nextTimes = updateTimes(nextTimes, "x", 1.f / rates.x());
                currentCell = updateCell(currentCell, "x", 1);
            } else {
                nextTimes = updateTimes(nextTimes, "x", -1.f / rates.x());
                currentCell = updateCell(currentCell, "x", -1);
            }
        } else if (nextTimes.y() == minTime) {
            if (rates.y() > 0.f) {
                nextTimes = updateTimes(nextTimes, "y", 1.f / rates.y());
                currentCell = updateCell(currentCell, "y", 1);
            } else {
                nextTimes = updateTimes(nextTimes, "y", -1.f / rates.y());
                currentCell = updateCell(currentCell, "y", -1);
            }
        } else if (nextTimes.z() == minTime) {
            if (rates.z() > 0.f) {
                nextTimes = updateTimes(nextTimes, "z", 1.f / rates.z());
                currentCell = updateCell(currentCell, "z", 1);
            } else {
                nextTimes = updateTimes(nextTimes, "z", -1.f / rates.z());
                currentCell = updateCell(currentCell, "z", -1);
            }
        } else { assert(0); }
    }
}
