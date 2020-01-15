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
        auto result = grid.findTransmittance(entryPoint, exitPoint, targetTransmittance);

        float expected = 1.f;
        REQUIRE(result.isValid);
        REQUIRE(result.distance == Approx(expected));
    }

    // target transmittance is equivalent to fraction of path
    {
        float expected = 0.3f;
        float targetTransmittance = std::exp(-0.4 * expected);
        auto result = grid.findTransmittance(entryPoint, exitPoint, targetTransmittance);

        REQUIRE(result.isValid);
        REQUIRE(result.distance == Approx(expected));
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
        auto result = grid.findTransmittance(entryPoint, exitPoint, targetTransmittance);

        float expected = 1.f;
        REQUIRE(result.isValid);
        REQUIRE(result.distance == Approx(expected));
    }

    // target transmittance is equivalent to fraction of path
    for (auto &grid : grids) {
        float expected = 0.3f;
        float targetTransmittance = std::exp(-sigmaT * expected);
        auto result = grid.findTransmittance(entryPoint, exitPoint, targetTransmittance);

        REQUIRE(result.isValid);
        REQUIRE(result.distance == Approx(expected));
    }
}

TEST_CASE("transmittance numbers based on grid extents", "[grid]") {
    const float sigmaT = 0.1;

    GridInfo gridInfo({
        1, 1, 1,
        -10.f, -10.f, -10.f,
        10.f, 10.f, 10.f
    });

    std::vector<float> gridData = { sigmaT };
    GridMedium grid(gridInfo, gridData);

    Point3 entryPoint(-10.f, 0.5f, 0.5f);
    Point3 exitPoint(10.f, 0.5f, 0.5f);

    // transmittance on straight line across
    {
        Color transmittance = grid.transmittance(entryPoint, exitPoint);

        float expected = std::exp(-sigmaT * 20.f);
        REQUIRE(transmittance.r() == Approx(expected));
    }

    // entry and exit can be switched
    {
        Color transmittance = grid.transmittance(exitPoint, entryPoint);

        float expected = std::exp(-sigmaT * 20.f);
        REQUIRE(transmittance.r() == Approx(expected));
    }

    // target transmittance is equivalent to full line across
    {
        float targetTransmittance = std::exp(-sigmaT * 20.f);
        auto result = grid.findTransmittance(entryPoint, exitPoint, targetTransmittance);

        float expected = 20.f;
        REQUIRE(result.isValid);
        REQUIRE(result.distance == Approx(expected));
    }

    // target transmittance is equivalent to fraction of path
    {
        float expected = 3.f;
        float targetTransmittance = std::exp(-sigmaT * expected);
        auto result = grid.findTransmittance(entryPoint, exitPoint, targetTransmittance);

        REQUIRE(result.isValid);
        REQUIRE(result.distance == Approx(expected));
    }
}

TEST_CASE("findTransmittance doesn't meet threshold, exits outside volume", "[grid]") {
    const float sigmaT = 0.4f;

    GridInfo gridInfo({
        1, 1, 1,
        0.f, 0.f, 0.f,
        1.f, 1.f, 1.f
    });

    std::vector<float> gridData = {sigmaT};
    GridMedium grid(gridInfo, gridData);

    Point3 entryPoint(0.f, 0.5f, 0.5f);
    Point3 exitPoint(1.f, 0.5f, 0.5f);

    // target transmittance is past full line across
    {
        float targetTransmittance = std::exp(-sigmaT * 1.1f);
        auto result = grid.findTransmittance(entryPoint, exitPoint, targetTransmittance);

        REQUIRE(!result.isValid);
        REQUIRE(result.distance == -1.f);
    }
}

TEST_CASE("findTransmittance doesn't meet threshold, exits inside volume", "[grid]") {
    const float sigmaT = 0.4f;

    GridInfo gridInfo({
        1, 1, 1,
        0.f, 0.f, 0.f,
        1.f, 1.f, 1.f
    });

    std::vector<float> gridData = {sigmaT};
    GridMedium grid(gridInfo, gridData);

    Point3 entryPoint(0.f, 0.5f, 0.5f);
    Point3 exitPoint(0.5f, 0.5f, 0.5f);

    // target transmittance is past full line across
    {
        float targetTransmittance = std::exp(-sigmaT * 0.8f);
        auto result = grid.findTransmittance(entryPoint, exitPoint, targetTransmittance);

        REQUIRE(!result.isValid);
        REQUIRE(result.distance == -1.f);
    }
}

TEST_CASE("transmittance handles non-intersection", "[grid]") {
    GridInfo gridInfo({
        1, 1, 1,
        0.f, 0.f, 0.f,
        1.f, 1.f, 1.f
    });

    std::vector<float> gridData = {0.4f};
    GridMedium grid(gridInfo, gridData);

    Point3 entryPoint(-1.f, -1.f, -1.f);
    Point3 exitPoint(-2.f, -2.f, -2.f);

    {
        Color transmittance = grid.transmittance(entryPoint, exitPoint);

        float expected = 1.f;
        REQUIRE(transmittance.r() == Approx(expected));
    }
}

TEST_CASE("transmittance handles unclamped start points", "[grid]") {
    float sigmaT = 0.4f;

    GridInfo gridInfo({
        1, 1, 1,
        0.f, 0.f, 0.f,
        1.f, 1.f, 1.f
    });

    std::vector<float> gridData = {sigmaT};
    GridMedium grid(gridInfo, gridData);

    Point3 entryPoint(-100.f, 0.5f, 0.5f);
    Point3 exitPoint(1.f, 0.5f, 0.5f);

    {
        Color transmittance = grid.transmittance(entryPoint, exitPoint);

        float expected = std::exp(-sigmaT * 1.f);
        REQUIRE(transmittance.r() == Approx(expected));
    }
}

TEST_CASE("transmittance handles unclamped end points", "[grid]") {
    float sigmaT = 0.4f;

    GridInfo gridInfo({
        1, 1, 1,
        0.f, 0.f, 0.f,
        1.f, 1.f, 1.f
    });

    std::vector<float> gridData = {sigmaT};
    GridMedium grid(gridInfo, gridData);

    Point3 entryPoint(0.f, 0.5f, 0.5f);
    Point3 exitPoint(100.f, 0.5f, 0.5f);

    {
        Color transmittance = grid.transmittance(entryPoint, exitPoint);

        float expected = std::exp(-sigmaT * 1.f);
        REQUIRE(transmittance.r() == Approx(expected));
    }
}

TEST_CASE("transmittance exit point inside the grid", "[grid]") {
    float sigmaT = 0.4f;

    GridInfo gridInfo({
        1, 1, 1,
        0.f, 0.f, 0.f,
        1.f, 1.f, 1.f
    });

    std::vector<float> gridData = {sigmaT};
    GridMedium grid(gridInfo, gridData);

    Point3 entryPoint(0.f, 0.5f, 0.5f);
    Point3 exitPoint(0.5f, 0.5f, 0.5f);

    {
        Color transmittance = grid.transmittance(entryPoint, exitPoint);

        float expected = std::exp(-sigmaT * 0.5f);
        REQUIRE(transmittance.r() == Approx(expected));
    }
}

TEST_CASE("transmittance start point inside the grid", "[grid]") {
    float sigmaT = 0.4f;

    GridInfo gridInfo({
        1, 1, 1,
        0.f, 0.f, 0.f,
        1.f, 1.f, 1.f
    });

    std::vector<float> gridData = {sigmaT};
    GridMedium grid(gridInfo, gridData);

    Point3 entryPoint(0.5f, 0.5f, 0.5f);
    Point3 exitPoint(1.f, 0.5f, 0.5f);

    {
        Color transmittance = grid.transmittance(entryPoint, exitPoint);

        float expected = std::exp(-sigmaT * 0.5f);
        REQUIRE(transmittance.r() == Approx(expected));
    }
}

TEST_CASE("1x1x1 transmittance fully inside the grid", "[grid]") {
    float sigmaT = 0.4f;

    GridInfo gridInfo({
        1, 1, 1,
        0.f, 0.f, 0.f,
        1.f, 1.f, 1.f
    });

    std::vector<float> gridData = {sigmaT};
    GridMedium grid(gridInfo, gridData);

    Point3 entryPoint(0.5f, 0.5f, 0.5f);
    Point3 exitPoint(0.5f, 0.6f, 0.5f);

    {
        Color transmittance = grid.transmittance(entryPoint, exitPoint);

        float expected = std::exp(-sigmaT * 0.1f);
        REQUIRE(transmittance.r() == Approx(expected));
    }
}

TEST_CASE("33x33x33 transmittance fully inside the grid", "[grid]") {
    float sigmaT = 0.4f;

    GridInfo gridInfo({
        33, 33, 33,
        0.f, 0.f, 0.f,
        1.f, 1.f, 1.f
    });

    std::vector<float> gridData(33 * 33 * 33, sigmaT);
    GridMedium grid(gridInfo, gridData);

    Point3 entryPoint(0.5f, 0.5f, 0.5f);
    Point3 exitPoint(0.5f, 0.6f, 0.5f);

    {
        Color transmittance = grid.transmittance(entryPoint, exitPoint);

        float expected = std::exp(-sigmaT * 0.1f);
        REQUIRE(transmittance.r() == Approx(expected));
    }
}
