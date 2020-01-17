#pragma once

#include "color.h"
#include "medium.h"
#include "point.h"
#include "random_generator.h"
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
    UniformGrid m_grid;

    float lookupSigmaT(int cellX, int cellY, int cellZ) const;
    Point3 worldToGrid(const Point3 &worldPoint) const;

    GridInfo m_gridInfo;

    float m_widthX;
    float m_widthY;
    float m_widthZ;

    float m_albedo;
    float m_scale;
};
