#pragma once

/* float trowbridgeLambda(float alphaX, float alphaY, const Vector3 &w) */
/* { */
/*     const float absTanTheta = fabsf(TangentFrame::tanTheta(wi)); */
/*     if (std::isinf(absTanTheta)) { */
/*         return Color(0.f); */
/*     } */

/*     const float alpha = sqrtf( */
/*         TangentFrame::cos2Phi(wi) * alphaX * alphaX */
/*         + TangentFrame::sin2Phi(wi) * alphaY * alphaY */
/*     ); */

/*     const float alpha2Tan2Theta = (alpha * absTanTheta) * (alpha * absTanTheta); */
/*     return (-1.f + sqrtf(1.f + alpha2Tan2Theta)) / 2.f; */
/* } */
