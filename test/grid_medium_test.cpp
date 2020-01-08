#include "color.h"
#include "grid_medium.h"
#include "point.h"
#include "vector.h"

#include "catch.hpp"

#include <cmath>

TEST_CASE("1x1x1 grid", "[grid]") {
    GridInfo gridInfo({
        1, 1, 1,
        0.f, 0.f, 0.f,
        1.f, 1.f, 1.f
    });

    std::vector<float> gridData = {0.4f};
    GridMedium grid(gridInfo, gridData);

    Point3 entryPoint(0.f, 0.5f, 0.5f);
    Point3 exitPoint(1.f, 0.5f, 0.5f);

    // transmittance on straight line across
    {
        Color transmittance = grid.transmittance(entryPoint, exitPoint);

        float expected = std::exp(-0.4f);
        REQUIRE(transmittance.r() == Approx(expected));
    }

    // entry and exit can be switched
    {
        Color transmittance = grid.transmittance(exitPoint, entryPoint);

        float expected = std::exp(-0.4f);
        REQUIRE(transmittance.r() == Approx(expected));
    }

    // target transmittance is equivalent to full line across
    {
        float targetTransmittance = std::exp(-0.4f);
        float distance = grid.findTransmittance(entryPoint, exitPoint, targetTransmittance);

        float expected = 1.f;
        REQUIRE(distance == Approx(expected));
    }

    // target transmittance is equivalent to fraction of path
    {
        float expected = 0.3f;
        float targetTransmittance = std::exp(-0.4 * expected);
        float distance = grid.findTransmittance(entryPoint, exitPoint, targetTransmittance);

        REQUIRE(distance == Approx(expected));
    }
}

TEST_CASE("grid resolution does not affect homogeneous calculations", "[grid]") {
    const float sigmaT = 0.6f;

    std::vector<GridMedium> grids;

    const int gridCount = 4;
    uint32_t gridSizes[gridCount][3] = {
        { 1, 1, 1 },
        { 2, 1, 1 },
        { 4, 4, 4 },
        { 20, 5, 20 },
    };
    for (int i = 0; i < gridCount; i++) {
        GridInfo gridInfo({
            gridSizes[i][0], gridSizes[i][1], gridSizes[i][1],
            0.f, 0.f, 0.f,
            1.f, 1.f, 1.f
        });

        std::vector<float> gridData(
            gridSizes[i][0] * gridSizes[i][1] *gridSizes[i][1],
            sigmaT
        );

        grids.push_back(GridMedium(gridInfo, gridData));
    }

    Point3 entryPoint(0.f, 0.5f, 0.5f);
    Point3 exitPoint(1.f, 0.5f, 0.5f);

    // transmittance on straight line across
    for (auto &grid : grids) {
        Color transmittance = grid.transmittance(entryPoint, exitPoint);

        float expected = std::exp(-sigmaT);
        REQUIRE(transmittance.r() == Approx(expected));
    }

    // target transmittance is equivalent to full line across
    for (auto &grid : grids) {
        float targetTransmittance = std::exp(-sigmaT) + 1e-6;
        float distance = grid.findTransmittance(entryPoint, exitPoint, targetTransmittance);

        float expected = 1.f;
        REQUIRE(distance == Approx(expected));
    }

    // target transmittance is equivalent to fraction of path
    for (auto &grid : grids) {
        float expected = 0.3f;
        float targetTransmittance = std::exp(-sigmaT * expected);
        float distance = grid.findTransmittance(entryPoint, exitPoint, targetTransmittance);

        REQUIRE(distance == Approx(expected));
    }
}
