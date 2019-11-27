#pragma once

#include "random_generator.h"

#include <vector>

class PhiCosThetaPDF {
public:
    PhiCosThetaPDF(int phiSteps, int cosThetaSteps);

    void splat(float phi, float theta, float value);
    void splatStepped(int phiStep, int cosThetaStep, float value);

    void build();

    float eval(float phi, float theta) const;
    void sample(RandomGenerator &random, float *phi, float *theta, float *pdf) const;

    float thetaAtStep(int cosThetaStep) const;
    float phiAtStep(int phiStep) const;

    void save() const;

private:
    bool m_built;

    int m_phiSteps;
    int m_cosThetaSteps;

    std::vector<float> m_map;
    std::vector<float> m_cdf;
};
