#pragma once

class PhiThetaPDF {
public:
    PhiThetaPDF(int phiSteps, int thetaSteps);

    void splat(float phi, float theta, float value);
    void build();

    float eval(float phi, float theta);
    void sample(float *phi, float *theta, float *pdf);

private:
    bool m_built;

    int m_phiSteps;
    int m_thetaSteps;
};
