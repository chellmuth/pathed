#pragma once

#include "tangent_frame.h"

#include <cmath>


float beckmanD(const float alpha, const Vector3 &wh)
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
