#pragma once

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
