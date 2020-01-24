#pragma once

#include "color.h"
#include "medium.h"
#include "point.h"
#include "random_generator.h"
#include "transform.h"
#include "uniform_grid.h"
#include "vector.h"

#include <cmath>
#include <ostream>
#include <vector>

class GridMedium : public Medium {
public:
    GridMedium(
        const GridInfo &gridInfo,
        const std::vector<float> &gridData,
        const Transform &worldToModel,
        float albedo,
        float scale
    );

    GridMedium(
        const GridInfo &gridInfo,
        const std::vector<float> &gridData
    ) : GridMedium(gridInfo, gridData, Transform(), 0.f, 1.f) {}

    Color transmittance(const Point3 &pointA, const Point3 &pointB) const override;

    TransmittanceQueryResult findTransmittance(
        const Point3 &entryPointWorld,
        const Point3 &exitPointWorld,
        float targetTransmittance
    ) const override;

    float sigmaT(const Point3 &worldPoint) const override;
    float sigmaS(const Point3 &worldPoint) const override;

    IntegrationResult integrate(
        const Point3 &entryPointWorld,
        const Point3 &exitPointWorld,
        const Scene &scene,
        RandomGenerator &random
    ) const override;

protected:
    enum class GridFrame {
        World,
        Model,
        Grid
    };

    float sigmaT(const Point3 &worldPoint, GridFrame frame) const;

    Transform m_worldToModel;
    UniformGrid m_grid;

    Point3 worldToModel(const Point3 &worldPoint) const;
    Point3 modelToGrid(const Point3 &modelPoint) const;
    Point3 modelToWorld(const Point3 &modelPoint) const;
    Point3 worldToGrid(const Point3 &worldPoint) const;

    GridInfo m_gridInfo;

    float m_widthX;
    float m_widthY;
    float m_widthZ;

    float m_albedo;
    float m_scale;
};
