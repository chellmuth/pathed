#pragma once

#include "color.h"

#include <assert.h>
#include <cmath>
#include <vector>

#define INV_TWO_PI 0.15915494309189533577f
#define INV_PI 0.3183098861837907f
#define M_TWO_PI 6.283185307179586f

typedef struct {
    bool hasRealSolutions;
    float solution1;
    float solution2;
} QuadraticSolution;


QuadraticSolution solveQuadratic(const float a, const float b, const float c);

float lerp(float x1, float x2, float t);

unsigned int binarySearchCDF(std::vector<float> cdf, float xi);

namespace util {
    const float eta = 1e-3; // todo: should be lower

    inline void AssertLEClose(float bound, float value) {
        assert(bound <= value + eta);
    }

    inline void AssertClose(float value, float target) {
        assert(value - eta <= target);
        assert(value <= target + eta);
    }

    inline void AssertBetween(float value, float lowest, float highest) {
        assert(value <= highest);
        assert(value >= lowest);
    }

    inline void AssertBetweenClose(float value, float lowest, float highest) {
        assert(value <= highest + eta);
        assert(value + eta >= lowest);
    }

    inline float clamp(float value, float lowest, float highest) {
        return std::min(highest, std::max(value, lowest));
    }

    inline float clampClose(float value, float lowest, float highest) {
        AssertBetweenClose(value, lowest, highest);
        return clamp(value, lowest, highest);
    }

    inline Color exp(const Color &color) {
        return Color(
            std::exp(color.r()),
            std::exp(color.g()),
            std::exp(color.b())
        );
    }

    inline float square(float x) {
        return x * x;
    }
};
