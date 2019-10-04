#include "phi_theta_pdf.h"

PhiThetaPDF::PhiThetaPDF(int phiSteps, int thetaSteps)
    : m_phiSteps(phiSteps), m_thetaSteps(thetaSteps), m_built(false)
{}

void PhiThetaPDF::splat(float phi, float theta, float value)
{
}

void PhiThetaPDF::build()
{
}

float PhiThetaPDF::eval(float phi, float theta)
{
    return 0.f;
}

void PhiThetaPDF::sample(float *phi, float *theta, float *pdf)
{
}

