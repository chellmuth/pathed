#pragma once

#include "random_generator.h"

#include <vector>

class PhiThetaPDF {
public:
    PhiThetaPDF(int phiSteps, int thetaSteps);

    void splat(float phi, float theta, float value);
    void build();

    float eval(float phi, float theta);
    void sample(RandomGenerator &random, float *phi, float *theta, float *pdf);

private:
    bool m_built;

    int m_phiSteps;
    int m_thetaSteps;

    std::vector<float> m_map;
    std::vector<float> m_cdf;
};
