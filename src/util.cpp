#include "math.h"

#include "util.h"

static QuadraticSolution solve(float a, float b)
{
    QuadraticSolution result = {
        .hasRealSolutions = true,
        .solution1 = a,
        .solution2 = b
    };
    return result;
}

static QuadraticSolution fail()
{
    QuadraticSolution result = {
        .hasRealSolutions = false,
        .solution1 = 0.f,
        .solution2 = 0.f
    };
    return result;
}

QuadraticSolution solveQuadratic(const float a, const float b, const float c)
{
    float discriminant = (b * b) - (4 * a * c);
    if (discriminant < 0.f) {
        return fail();
    }

    float solution1 = (-b + sqrt(discriminant)) / (2 * a);
    float solution2 = (-b - sqrt(discriminant)) / (2 * a);
    if (solution1 < solution2) {
        return solve(solution1, solution2);
    } else {
        return solve(solution2, solution1);
    }
}

float lerp(float x1, float x2, float t)
{
    return (1.f - t) * x1 + t * x2;
}
