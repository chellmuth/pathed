#pragma once

#define INV_TWO_PI 0.15915494309189533577f
#define M_TWO_PI 6.283185307179586

typedef struct {
    bool hasRealSolutions;
    float solution1;
    float solution2;
} QuadraticSolution;


QuadraticSolution solveQuadratic(const float a, const float b, const float c);
