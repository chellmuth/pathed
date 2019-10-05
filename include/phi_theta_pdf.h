#pragma once

#include "random_generator.h"

#include <vector>

class PhiThetaPDF {
public:
    PhiThetaPDF(int phiSteps, int thetaSteps);

    void splat(float phi, float theta, float value);
    void splatStepped(int phiStep, int thetaStep, float value);

    void build();

    float eval(float phi, float theta) const;
    void sample(RandomGenerator &random, float *phi, float *theta, float *pdf) const;

    float thetaAtStep(int thetaStep) const;
    float phiAtStep(int phiStep) const;

    void save() const;

private:
    bool m_built;

    int m_phiSteps;
    int m_thetaSteps;

    std::vector<float> m_map;
    std::vector<float> m_cdf;
};
