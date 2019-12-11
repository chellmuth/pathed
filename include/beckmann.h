#pragma once

#include "tangent_frame.h"

#include <cmath>

float beckmannD(const float alpha, const Vector3 &wh)
{
    const float tan2Theta = TangentFrame::tan2Theta(wh);
    if (std::isinf(tan2Theta)) { return 0.f; }

    const float cos2Theta = TangentFrame::cos2Theta(wh);
    const float cos4Theta = cos2Theta * cos2Theta;
    const float alpha2 = alpha * alpha;

    const float numerator = std::exp(
        -tan2Theta * (
            TangentFrame::cos2Phi(wh) / alpha2
            + TangentFrame::sin2Phi(wh) / alpha2
        )
    );
    const float denominator = M_PI * alpha2 * cos2Theta;

    return numerator / denominator;
}

float beckmannLambda(float alphaX, float alphaY, const Vector3 &w)
{
    const float absTanTheta = std::abs(TangentFrame::tanTheta(w));
    if (std::isinf(absTanTheta)) { return 0.f; }

    const float alpha = sqrtf(
        TangentFrame::cos2Phi(w) * alphaX * alphaX
        + TangentFrame::sin2Phi(w) * alphaY * alphaY
    );

    const float a = 1.f / (alpha * absTanTheta);

    if (a >= 1.6f) { return 0.f; }

    return (1 - 1.259f * a + 0.396f * a * a)
        / (3.535f * a + 2.181f * a * a);
}

float G1(float alphaX, float alphaY, const Vector3 &w)
{
    return 1.f / (1.f + beckmannLambda(alphaX, alphaY, w));
}
