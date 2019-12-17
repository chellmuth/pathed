#pragma once

#include "vector.h"

void cartesianToSpherical(Vector3 cartesian, float *phi, float *theta);
Vector3 sphericalToCartesian(float phi, float theta);
Vector3 sphericalToCartesian(float phi, float cosTheta, float sinTheta);
