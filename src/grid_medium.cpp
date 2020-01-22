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
    const Transform &worldToModel,
    float albedo,
    float scale
) : m_gridInfo(gridInfo),
    m_grid(
        gridInfo.cellsX,
        gridInfo.cellsY,
        gridInfo.cellsZ,
        gridData
    ),
    m_worldToModel(worldToModel),
    m_albedo(albedo),
    m_scale(scale)
{
    m_widthX = m_gridInfo.widthX();
    m_widthY = m_gridInfo.widthY();
    m_widthZ = m_gridInfo.widthZ();
}

float GridMedium::sigmaT(const Point3 &point, GridFrame frame) const
{
    Point3 gridPoint(0.f, 0.f, 0.f);

    switch (frame) {
    case GridFrame::World: { gridPoint = worldToGrid(point); break; }
    case GridFrame::Model: { gridPoint = modelToGrid(point); break; }
    case GridFrame::Grid: { gridPoint = point; break; }
    }

    return m_grid.interpolate(gridPoint) * m_scale;
}

float GridMedium::sigmaT(const Point3 &worldPoint) const
{
    return sigmaT(worldPoint, GridFrame::World);
}

float GridMedium::sigmaS(const Point3 &worldPoint) const
{
    const Point3 gridPoint = worldToGrid(worldPoint);
    return m_grid.interpolate(gridPoint) * m_scale * m_albedo;
}

Point3 GridMedium::worldToGrid(const Point3 &worldPoint) const
{
    const Point3 modelPoint = worldToModel(worldPoint);
    return modelToGrid(modelPoint);
}

Point3 GridMedium::modelToGrid(const Point3 &modelPoint) const
{
    return Point3(
        ((modelPoint.x() - m_gridInfo.minX) / m_widthX) * (m_gridInfo.cellsX - 1),
        ((modelPoint.y() - m_gridInfo.minY) / m_widthY) * (m_gridInfo.cellsY - 1),
        ((modelPoint.z() - m_gridInfo.minZ) / m_widthZ) * (m_gridInfo.cellsZ - 1)
    );
}

Point3 GridMedium::modelToWorld(const Point3 &worldPoint) const
{
    return m_worldToModel.applyInverse(worldPoint);
}

Point3 GridMedium::worldToModel(const Point3 &worldPoint) const
{
    return m_worldToModel.apply(worldPoint);
}

Color GridMedium::transmittance(const Point3 &entryPointWorld, const Point3 &exitPointWorld) const
{
    AABB aabb(
        m_gridInfo.minX, m_gridInfo.minY, m_gridInfo.minZ,
        m_gridInfo.maxX, m_gridInfo.maxY, m_gridInfo.maxZ
    );
    AABBHit hit = aabb.intersect(worldToModel(entryPointWorld), worldToModel(exitPointWorld));
    if (!hit.isHit) { return Color(1.f); }

    const Point3 entryPoint = modelToGrid(hit.enterPoint);
    const Point3 exitPoint = modelToGrid(hit.exitPoint);

    float accumulatedExponent = 0.f;

    RegularTrackerState trackerState(m_gridInfo, entryPoint, exitPoint);

    const Ray trackerRay(
        hit.enterPoint,
        (hit.exitPoint - hit.enterPoint).toVector().normalized()
    );

    auto stepResult = trackerState.step();
    while(stepResult.isValidStep) {
        const float midpointTime = (stepResult.enterTime + stepResult.currentTime) / 2.f;
        const Point3 midpointModel = trackerRay.at(midpointTime);
        const float midpointSigmaT = sigmaT(midpointModel, GridFrame::Model);

        accumulatedExponent += midpointSigmaT * stepResult.cellTime;

        if (DEBUG) {
            std::cout << "[transmittance] midpointModel: " << midpointModel.toString() << std::endl;
            std::cout << "[transmittance] cell time: " << stepResult.cellTime << " sigmaT: " << midpointSigmaT << std::endl;
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
        const float midpointSigmaT = sigmaT(midpointWorld);

        const float cellExponent = midpointSigmaT * stepResult.cellTime;
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
    const Point3 entryPointModel = worldToModel(entryPointWorld);
    const Point3 exitPointModel = worldToModel(exitPointWorld);

    const Vector3 travelVector = (exitPointModel - entryPointModel).toVector();
    const Ray travelRay(entryPointModel, travelVector.normalized());

    AABB aabb(
        m_gridInfo.minX, m_gridInfo.minY, m_gridInfo.minZ,
        m_gridInfo.maxX, m_gridInfo.maxY, m_gridInfo.maxZ
    );
    AABBHit hit = aabb.intersect(travelRay);
    if (!hit.isHit) { return Color(0.f); }

    const float targetTransmittance = random.next();
    const TransmittanceQueryResult queryResult = findTransmittance(
        modelToWorld(hit.enterPoint),
        modelToWorld(hit.exitPoint),
        targetTransmittance
    );

    if (!queryResult.isValid) { return Color(0.f); }
    const Point3 samplePoint = modelToWorld(travelRay.at(queryResult.distance));

    const Color Ld = VolumeHelper::directSampleLights(*this, samplePoint, scene, random);

    // full estimator is Ld * Tr * sigma_s / p(x)
    // = Ld * Tr * sigma_s / (Tr * sigma_t)
    // = Ld * (sigma_s / sigma_t)
    // = Ld * albedo

    return Ld * m_albedo;
}
