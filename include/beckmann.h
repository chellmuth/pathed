#pragma once

#include "vector.h"

float beckmannD(const float alpha, const Vector3 &wh);
float beckmannLambda(float alphaX, float alphaY, const Vector3 &w);
float beckmannG(float alphaX, float alphaY, const Vector3 &wi, const Vector3 &wo);
