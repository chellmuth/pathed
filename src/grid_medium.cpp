#include "grid_medium.h"

#include "aabb.h"
#include "regular_tracker.h"
#include "scene.h"
#include "volume_helper.h"
#include "util.h"

#include <assert.h>
#include <limits>

static const bool DEBUG = false;


GridMedium::GridMedium(
    const GridInfo &gridInfo,
    const std::vector<float> &gridData,
    float albedo,
    float scale
) : m_gridInfo(gridInfo),
    m_grid(
        gridInfo.cellsX,
        gridInfo.cellsY,
        gridInfo.cellsZ,
        gridData
    ),
    m_albedo(albedo),
    m_scale(scale)
{
    m_widthX = m_gridInfo.widthX();
    m_widthY = m_gridInfo.widthY();
    m_widthZ = m_gridInfo.widthZ();
}

float GridMedium::sigmaT(const Point3 &worldPoint) const
{
    const Point3 gridPoint = worldToGrid(worldPoint);
    return m_grid.interpolate(gridPoint) * m_scale;
}

float GridMedium::sigmaS(const Point3 &worldPoint) const
{
    const Point3 gridPoint = worldToGrid(worldPoint);
    return m_grid.interpolate(gridPoint) * m_scale * m_albedo;
}

Point3 GridMedium::worldToGrid(const Point3 &worldPoint) const
{
    return Point3(
        ((worldPoint.x() - m_gridInfo.minX) / m_widthX) * (m_gridInfo.cellsX - 1),
        ((worldPoint.y() - m_gridInfo.minY) / m_widthY) * (m_gridInfo.cellsY - 1),
        ((worldPoint.z() - m_gridInfo.minZ) / m_widthZ) * (m_gridInfo.cellsZ - 1)
    );
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

    const Ray trackerRay(
        entryPointWorld,
        (exitPointWorld - entryPointWorld).toVector().normalized()
    );

    auto stepResult = trackerState.step();
    while(stepResult.isValidStep) {
        // const GridCell &currentCell = stepResult.cell;
        // const float sigmaT = lookupSigmaT(currentCell.x, currentCell.y, currentCell.z);

        const float midpointTime = (stepResult.enterTime + stepResult.currentTime) / 2.f;
        const Point3 midpointWorld = trackerRay.at(midpointTime);
        const Point3 midpoint = worldToGrid(midpointWorld);
        const float sigmaT = m_grid.interpolate(midpoint) * m_scale;

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

    const Ray trackerRay(
        entryPointWorld,
        (exitPointWorld - entryPointWorld).toVector().normalized()
    );

    auto stepResult = trackerState.step();
    while(stepResult.isValidStep) {
        const float midpointTime = (stepResult.enterTime + stepResult.currentTime) / 2.f;
        const Point3 midpointWorld = trackerRay.at(midpointTime);
        const Point3 midpoint = worldToGrid(midpointWorld);
        const float sigmaT = m_grid.interpolate(midpoint) * m_scale;

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
    return Ld;
}
