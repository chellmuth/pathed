#pragma once

#include "random_generator.h"

#include <vector>

class Distribution {
public:
    Distribution(const std::vector<float> &values);

    int sample(float *pdf, RandomGenerator &random) const;
    float pdf(int index) const;

private:
    std::vector<float> m_cdf;
};
