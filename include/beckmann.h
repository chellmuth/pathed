#pragma once

#include "random_generator.h"
#include "vector.h"

Vector3 beckmannSampleWh(float alpha, const Vector3 &wo, RandomGenerator &random);
float beckmannPDF(float alpha, const Vector3 &wh);

float beckmannD(const float alpha, const Vector3 &wh);
float beckmannLambda(float alphaX, float alphaY, const Vector3 &w);
float beckmannG(float alphaX, float alphaY, const Vector3 &wo, const Vector3 &wi);
