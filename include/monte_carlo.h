#pragma once

#include "random_generator.h"
#include "vector.h"

Vector3 UniformSampleHemisphere(RandomGenerator &random);
float UniformHemispherePdf(const Vector3 &v);

Vector3 CosineSampleHemisphere(RandomGenerator &random);
float CosineHemispherePdf(const Vector3 &v);
