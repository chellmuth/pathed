#include "grid_medium.h"

#include "aabb.h"
#include "scene.h"
#include "volume_helper.h"
#include "util.h"

#include <assert.h>
#include <limits>

static const bool DEBUG = false;

std::ostream &operator<<(std::ostream &os, const GridCell &gridCell)
{
    return os << "[GridCell: "
              << "x=" << gridCell.x << " "
              << "y=" << gridCell.y << " "
              << "z=" << gridCell.z << "]" << std::endl;
}

GridMedium::GridMedium(
    const GridInfo &gridInfo,
    const std::vector<float> &gridData,
    float albedo,
    float scale
) : m_gridInfo(gridInfo),
    m_gridData(gridData),
    m_albedo(albedo),
    m_scale(scale)
{
    m_widthX = m_gridInfo.widthX();
    m_widthY = m_gridInfo.widthY();
    m_widthZ = m_gridInfo.widthZ();
}

float GridMedium::lookupSigmaT(int cellX, int cellY, int cellZ) const
{
    if (cellX < 0 || cellX >= m_gridInfo.cellsX) { return 0.f; }
    if (cellY < 0 || cellY >= m_gridInfo.cellsY) { return 0.f; }
    if (cellZ < 0 || cellZ >= m_gridInfo.cellsZ) { return 0.f; }

    const int index = (cellZ * m_gridInfo.cellsY + cellY) * m_gridInfo.cellsX + cellX;
    const float density = m_gridData[index];

    return density * m_scale;
}

float GridMedium::interpolate(Point3 gridPoint) const
{
    return interpolate(gridPoint.x(), gridPoint.y(), gridPoint.z());
}

float GridMedium::interpolate(float gridX, float gridY, float gridZ) const
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

    const float c000 = lookupSigmaT(x0, y0, z0);
    const float c001 = lookupSigmaT(x0, y0, z1);
    const float c010 = lookupSigmaT(x0, y1, z0);
    const float c011 = lookupSigmaT(x0, y1, z1);
    const float c100 = lookupSigmaT(x1, y0, z0);
    const float c101 = lookupSigmaT(x1, y0, z1);
    const float c110 = lookupSigmaT(x1, y1, z0);
    const float c111 = lookupSigmaT(x1, y1, z1);

    const float c_00 = c000 * (1 - xd) + c100 * xd;
    const float c_01 = c001 * (1 - xd) + c101 * xd;
    const float c_10 = c010 * (1 - xd) + c110 * xd;
    const float c_11 = c011 * (1 - xd) + c111 * xd;

    const float c__0 = c_00 * (1 - yd) + c_10 * yd;
    const float c__1 = c_01 * (1 - yd) + c_11 * yd;

    const float c = c__0 * (1 - zd) + c__1 * zd;
    return c;
}

float GridMedium::sigmaT(const Point3 &worldPoint) const
{
    const Point3 gridPoint = worldToGrid(worldPoint);
    return interpolate(gridPoint);
}

float GridMedium::sigmaS(const Point3 &worldPoint) const
{
    const Point3 gridPoint = worldToGrid(worldPoint);
    return interpolate(gridPoint);
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

Point3 GridMedium::worldToGrid(const Point3 &worldPoint) const
{
    return Point3(
        ((worldPoint.x() - m_gridInfo.minX) / m_widthX) * m_gridInfo.cellsX,
        ((worldPoint.y() - m_gridInfo.minY) / m_widthY) * m_gridInfo.cellsY,
        ((worldPoint.z() - m_gridInfo.minZ) / m_widthZ) * m_gridInfo.cellsZ
    );
}

static bool validCell(const GridInfo &gridInfo, const GridCell &cell)
{
    if (cell.x < 0 || cell.y < 0 || cell.z < 0) { return false; }
    if (cell.x >= gridInfo.cellsX) { return false; }
    if (cell.y >= gridInfo.cellsY) { return false; }
    if (cell.z >= gridInfo.cellsZ) { return false; }

    return true;
}

RegularTrackerState::RegularTrackerState(const GridInfo &gridInfo, const Point3& entryPoint, const Point3 &exitPoint)
    : m_gridInfo(gridInfo),
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
    m_currentTime = clippedTime;

    if (DEBUG) {
        std::cout << "[step] steppedCell: " << steppedCell << std::endl;
        std::cout << "[step] cell time: " << cellTime << " world time: " << worldTime(cellTime) << std::endl;
    }

    return RegularTrackerStepResult({
        true,
        steppedCell,
        worldTime(cellTime),
        worldTime(m_currentTime)
    });
}

Color GridMedium::transmittance(const Point3 &entryPointWorld, const Point3 &exitPointWorld) const
{
    AABB aabb(
        m_gridInfo.minX, m_gridInfo.minY, m_gridInfo.minZ,
        m_gridInfo.maxX, m_gridInfo.maxY, m_gridInfo.maxZ
    );
    AABBHit hit = aabb.intersect(entryPointWorld, exitPointWorld);
    if (!hit.isHit) { return Color(1.f); }

    const Point3 entryPoint = worldToGrid(hit.enterPoint);
    const Point3 exitPoint = worldToGrid(hit.exitPoint);

    float accumulatedExponent = 0.f;

    RegularTrackerState trackerState(m_gridInfo, entryPoint, exitPoint);

    auto stepResult = trackerState.step();
    while(stepResult.isValidStep) {
        const GridCell &currentCell = stepResult.cell;
        const float sigmaT = lookupSigmaT(currentCell.x, currentCell.y, currentCell.z);

        accumulatedExponent += sigmaT * stepResult.cellTime;

        if (DEBUG) {
            std::cout << "[transmittance] cell time: " << stepResult.cellTime << " sigmaT: " << sigmaT << std::endl;
        }

        stepResult = trackerState.step();
    }

    return util::exp(-accumulatedExponent);
}

TransmittanceQueryResult GridMedium::findTransmittance(
    const Point3 &entryPointWorld,
    const Point3 &exitPointWorld,
    float targetTransmittance
) const {
    const float targetExponent = -std::log(targetTransmittance);

    const Point3 entryPoint = worldToGrid(entryPointWorld);
    const Point3 exitPoint = worldToGrid(exitPointWorld);

    float accumulatedExponent = 0.f;

    RegularTrackerState trackerState(m_gridInfo, entryPoint, exitPoint);

    auto stepResult = trackerState.step();
    while(stepResult.isValidStep) {
        const GridCell &currentCell = stepResult.cell;
        const float sigmaT = lookupSigmaT(currentCell.x, currentCell.y, currentCell.z);

        const float cellExponent = sigmaT * stepResult.cellTime;
        accumulatedExponent += cellExponent;

        if (accumulatedExponent >= targetExponent) {
            const float overflow = accumulatedExponent - targetExponent;
            const float cellRatio = 1.f - overflow / cellExponent;
            const float actualCellTime = stepResult.cellTime * cellRatio;

            return TransmittanceQueryResult({
                true,
                stepResult.currentTime - stepResult.cellTime + actualCellTime
            });
        }

        stepResult = trackerState.step();
    }

    return TransmittanceQueryResult({ false, -1.f });
}

Color GridMedium::integrate(
    const Point3 &entryPointWorld,
    const Point3 &exitPointWorld,
    const Scene &scene,
    RandomGenerator &random
) const
{
    const Vector3 travelVector = (exitPointWorld - entryPointWorld).toVector();
    const Ray travelRay(entryPointWorld, travelVector.normalized());

    AABB aabb(
        m_gridInfo.minX, m_gridInfo.minY, m_gridInfo.minZ,
        m_gridInfo.maxX, m_gridInfo.maxY, m_gridInfo.maxZ
    );
    AABBHit hit = aabb.intersect(travelRay);
    if (!hit.isHit) { return Color(0.f); }

    const float targetTransmittance = random.next();
    const TransmittanceQueryResult queryResult = findTransmittance(
        hit.enterPoint,
        hit.exitPoint,
        targetTransmittance
    );

    if (!queryResult.isValid) { return Color(0.f); }
    const Point3 samplePoint = travelRay.at(queryResult.distance);

    const Color Ld = VolumeHelper::directSampleLights(*this, samplePoint, scene, random);
    return Ld * sigmaS(samplePoint);
}
