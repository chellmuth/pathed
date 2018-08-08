#pragma once

#include "random_generator.h"
#include "vector.h"

Vector3 UniformSampleHemisphere(RandomGenerator &random);
float UniformHemispherePdf();
