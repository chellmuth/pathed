#include "phi_cos_theta_pdf.h"

#include "image.h"
#include "util.h"

#include <assert.h>
#include <cmath>

PhiCosThetaPDF::PhiCosThetaPDF(int phiSteps, int cosThetaSteps)
    : m_phiSteps(phiSteps), m_cosThetaSteps(cosThetaSteps), m_built(false)
{
    const int length = m_phiSteps * m_cosThetaSteps;

    m_map.resize(length);
    for (int i = 0; i < length; i++) {
        m_map[i] = 0.f;
    }

    m_cdf.resize(length);
    for (int i = 0; i < length; i++) {
        m_cdf[i] = 0.f;
    }
}

static int getPhiStep(float phi, int phiSteps)
{
    return floorf((phi / M_TWO_PI) * phiSteps);
}

static int getCosThetaStep(float theta, int cosThetaSteps)
{
    return (int)floorf(cosf(theta) * cosThetaSteps);
}

static int getIndexStepped(int phiStep, int cosThetaStep, int phiSteps)
{
    return cosThetaStep * phiSteps + phiStep;
}

static int getIndex(float phi, float theta, int phiSteps, int cosThetaSteps)
{
    const int phiStep = getPhiStep(phi, phiSteps);
    const int cosThetaStep = getCosThetaStep(theta, cosThetaSteps);

    return getIndexStepped(phiStep, cosThetaStep, phiSteps);
}

static void getStepsFromIndex(int index, int *phiStep, int *cosThetaStep, int phiSteps)
{
    *phiStep = index % phiSteps;
    *cosThetaStep = (int)floorf(index / phiSteps);
}

float PhiCosThetaPDF::thetaAtStep(int cosThetaStep) const
{
    const float cosTheta = (cosThetaStep + 0.5f) / m_cosThetaSteps;
    return acosf(cosTheta);
}

float PhiCosThetaPDF::phiAtStep(int phiStep) const
{
    return M_TWO_PI * (phiStep + 0.5f) / m_phiSteps;
}

void PhiCosThetaPDF::splatStepped(int phiStep, int cosThetaStep, float value)
{
    const int index = getIndexStepped(phiStep, cosThetaStep, m_phiSteps);

    m_map[index] += value;
}

void PhiCosThetaPDF::splat(float phi, float theta, float value)
{
    const int index = getIndex(phi, theta, m_phiSteps, m_cosThetaSteps);

    m_map[index] += value;
}

void PhiCosThetaPDF::build()
{
    if (m_built) { return; }

    const int length = m_map.size();

    float sum = 0.f;
    for (int i = 0; i < length; i++) {
        sum += m_map[i];
    }
    assert(sum > 0.f);

    for (int i = 0; i < length; i++) {
        m_cdf[i] = m_map[i] / sum;
        if (i > 0) {
            m_cdf[i] += m_cdf[i - 1];
        }
    }
    assert(fabsf(m_cdf[length - 1] - 1.f) < 1e-5);

    m_cdf[length - 1] = 1.f;

    m_built = true;
}

float PhiCosThetaPDF::eval(float phi, float theta) const
{
    assert(m_built);

    const int index = getIndex(phi, theta, m_phiSteps, m_cosThetaSteps);
    if (index > 0) {
        return m_cdf[index] - m_cdf[index] - 1;
    } else {
        return m_cdf[index];
    }
}

void PhiCosThetaPDF::sample(RandomGenerator &random, float *phi, float *theta, float *pdf) const
{
    assert(m_built);

    const float xi = random.next();

    const int length = m_map.size();
    for (int i = 0; i < i < length; i++) {
        if (xi > m_cdf[i]) {
            continue;
        }

        int phiStep, cosThetaStep;
        getStepsFromIndex(i, &phiStep, &cosThetaStep, m_phiSteps);

        float phi1 = M_TWO_PI * (1.f * phiStep / m_phiSteps);
        float phi2 = M_TWO_PI * (1.f * (phiStep + 1) / m_phiSteps);

        float cosTheta1 = 1.f * cosThetaStep / m_cosThetaSteps;
        float cosTheta2 = 1.f * (cosThetaStep + 1) / m_cosThetaSteps;

        assert(phiStep != -1);
        assert(cosThetaStep != -1);

        const float xiPhi = random.next();
        *phi = lerp(phi1, phi2, xiPhi);

        const float xiCosTheta = random.next();
        const float cosThetaSample = lerp(cosTheta1, cosTheta2, xiCosTheta);
        *theta = acosf(cosThetaSample);

        float massRatio = m_cdf[i];
        if (i > 0) {
            massRatio -= m_cdf[i - 1];
        }

        *pdf = massRatio / ((cosTheta2 - cosTheta1) * (phi2 - phi1));
        return;
    }

    assert(0);
}

void PhiCosThetaPDF::save() const
{
    Image image(m_phiSteps, m_cosThetaSteps);
    for (int cosThetaStep = 0; cosThetaStep < m_cosThetaSteps; cosThetaStep++) {
        for (int phiStep = 0; phiStep < m_phiSteps; phiStep++) {
            int index = getIndexStepped(phiStep, cosThetaStep, m_phiSteps);

            float value = m_map[index];
            image.set(m_cosThetaSteps - cosThetaStep - 1, phiStep, value, value, value);
        }
    }

    image.save("pdf_cos-theta");
}
