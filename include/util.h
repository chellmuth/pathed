#pragma once

#include <assert.h>
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
    inline void AssertLEClose(float bound, float value) {
        const float eta = 0.5f; // todo: this should be closer to 1e-5
        assert(bound <= value + eta);
    }

    inline float clamp(float value, float lowest, float highest) {
        return std::min(highest, std::max(value, lowest));
    }

    inline float clampClose(float value, float lowest, float highest) {
        AssertLEClose(lowest, value);
        AssertLEClose(value, highest);

        return clamp(value, lowest, highest);
    }
};
