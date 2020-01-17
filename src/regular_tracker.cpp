#include "regular_tracker.h"

#include <assert.h>
#include <iostream>

static const bool DEBUG = false;

std::ostream &operator<<(std::ostream &os, const GridCell &gridCell)
{
    return os << "[GridCell: "
              << "x=" << gridCell.x << " "
              << "y=" << gridCell.y << " "
              << "z=" << gridCell.z << "]" << std::endl;
}

static Point3 gridToWorld(const GridInfo &gridInfo, const Point3 &gridPoint)
{
    return Point3(
        (gridPoint.x() / gridInfo.cellsX) * gridInfo.widthX() + gridInfo.minX,
        (gridPoint.y() / gridInfo.cellsY) * gridInfo.widthY() + gridInfo.minY,
        (gridPoint.z() / gridInfo.cellsZ) * gridInfo.widthZ() + gridInfo.minZ
    );
}

static float calculateNextDistance(float currentValue, bool isForward)
{
    if (isForward) {
        if (currentValue == std::floor(currentValue)) {
            return 1.f;
        } else {
            return std::ceil(currentValue) - currentValue;
        }
    } else {
        if (currentValue == std::floor(currentValue)) {
            return -1.f;
        } else {
            return std::floor(currentValue) - currentValue;
        }
    }
}

static float calculateNextTime(float nextDistance, float rate)
{
    if (rate == 0.f) { return std::numeric_limits<float>::max(); }
    return nextDistance / rate;
}

static Vector3 calculateNextTimes(const Vector3 &rates, const Point3 &entryPoint)
{
    const Vector3 nextDistances(
        calculateNextDistance(entryPoint.x(), rates.x() > 0.f),
        calculateNextDistance(entryPoint.y(), rates.y() > 0.f),
        calculateNextDistance(entryPoint.z(), rates.z() > 0.f)
    );

    return Vector3(
        calculateNextTime(nextDistances.x(), rates.x()),
        calculateNextTime(nextDistances.y(), rates.y()),
        calculateNextTime(nextDistances.z(), rates.z())
    );
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

RegularTrackerState::RegularTrackerState(
    const GridInfo &gridInfo,
    const Point3& entryPoint,
    const Point3 &exitPoint
) : m_gridInfo(gridInfo),
    m_entryPoint(entryPoint),
    m_exitPoint(exitPoint),
    m_currentCell({-1, -1, -1}),
    m_rates(0.f),
    m_nextTimes(0.f)
{
    const Vector3 rayPath = (m_exitPoint - m_entryPoint).toVector();
    const float totalDistance = rayPath.length();

    const Vector3 spans = (m_exitPoint - m_entryPoint).toVector();
    m_rates = spans / totalDistance;

    m_nextTimes = calculateNextTimes(m_rates, m_entryPoint);

    m_currentTime = 0.f;
    m_currentCell = GridCell(m_entryPoint, gridInfo);

    m_endTime = totalDistance;

    if (DEBUG) {
        std::cout << "[constructor] entry point: " << m_entryPoint.toString() << std::endl;
        std::cout << "[constructor] exit point: " << m_exitPoint.toString() << std::endl;
    }
}

float RegularTrackerState::worldTime(float gridTime)
{
    const float totalGridTime = (m_entryPoint - m_exitPoint).toVector().length();
    const float timeRatio = gridTime / totalGridTime;

    const float totalWorldTime = (gridToWorld(m_gridInfo, m_exitPoint) - gridToWorld(m_gridInfo, m_entryPoint)).toVector().length();

    return timeRatio * totalWorldTime;
}

RegularTrackerStepResult RegularTrackerState::step()
{
    if (m_currentTime >= m_endTime) {
        return RegularTrackerStepResult({
            false,
            GridCell({-1, -1, -1}),
            0.f,
            0.f,
            0.f
        });
    }

    const GridCell steppedCell = m_currentCell;

    const float minTime = m_nextTimes.min();

    if (m_nextTimes.x() == minTime) {
        if (m_rates.x() > 0.f) {
            m_nextTimes = updateTimes(m_nextTimes, "x", 1.f / m_rates.x());
            m_currentCell = updateCell(m_currentCell, "x", 1);
        } else {
            m_nextTimes = updateTimes(m_nextTimes, "x", -1.f / m_rates.x());
            m_currentCell = updateCell(m_currentCell, "x", -1);
        }
    } else if (m_nextTimes.y() == minTime) {
        if (m_rates.y() > 0.f) {
            m_nextTimes = updateTimes(m_nextTimes, "y", 1.f / m_rates.y());
            m_currentCell = updateCell(m_currentCell, "y", 1);
        } else {
            m_nextTimes = updateTimes(m_nextTimes, "y", -1.f / m_rates.y());
            m_currentCell = updateCell(m_currentCell, "y", -1);
        }
    } else if (m_nextTimes.z() == minTime) {
        if (m_rates.z() > 0.f) {
            m_nextTimes = updateTimes(m_nextTimes, "z", 1.f / m_rates.z());
            m_currentCell = updateCell(m_currentCell, "z", 1);
        } else {
            m_nextTimes = updateTimes(m_nextTimes, "z", -1.f / m_rates.z());
            m_currentCell = updateCell(m_currentCell, "z", -1);
        }
    } else { assert(0); }

    const float clippedTime = std::min(minTime, m_endTime);
    const float cellTime = clippedTime - m_currentTime;

    const float enterTime = m_currentTime;

    m_currentTime = clippedTime;

    if (DEBUG) {
        std::cout << "[step] steppedCell: " << steppedCell << std::endl;
        std::cout << "[step] cell time: " << cellTime << " world time: " << worldTime(cellTime) << std::endl;
    }

    return RegularTrackerStepResult({
        true,
        steppedCell,
        worldTime(cellTime),
        worldTime(enterTime),
        worldTime(m_currentTime)
    });
}
