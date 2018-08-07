#pragma once

#define INV_TWOPI 0.15915494309189533577f

typedef struct {
    bool hasRealSolutions;
    float solution1;
    float solution2;
} QuadraticSolution;


QuadraticSolution solveQuadratic(const float a, const float b, const float c);
