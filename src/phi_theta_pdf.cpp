#include "phi_theta_pdf.h"

#include "image.h"
#include "util.h"

#include <assert.h>
#include <cmath>

PhiThetaPDF::PhiThetaPDF(int phiSteps, int thetaSteps)
    : m_phiSteps(phiSteps), m_thetaSteps(thetaSteps), m_built(false)
{
    const int length = m_phiSteps * m_thetaSteps;

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

static int getThetaStep(float theta, int thetaSteps)
{
    return (int)floorf((theta / (M_PI / 2.f)) * thetaSteps);
}

static int getIndexStepped(int phiStep, int thetaStep, int phiSteps)
{
    return thetaStep * phiSteps + phiStep;
}

static int getIndex(float phi, float theta, int phiSteps, int thetaSteps)
{
    const int phiStep = getPhiStep(phi, phiSteps);
    const int thetaStep = getThetaStep(theta, thetaSteps);

    return getIndexStepped(phiStep, thetaStep, phiSteps);
}

static void getStepsFromIndex(int index, int *phiStep, int *thetaStep, int phiSteps)
{
    *phiStep = index % phiSteps;
    *thetaStep = (int)floorf(index / phiSteps);
}

float PhiThetaPDF::thetaAtStep(int thetaStep) const
{
    return (M_PI / 2.f) * (thetaStep + 0.5f) / m_thetaSteps;
}

float PhiThetaPDF::phiAtStep(int phiStep) const
{
    return M_TWO_PI * (phiStep + 0.5f) / m_phiSteps;
}

void PhiThetaPDF::splatStepped(int phiStep, int thetaStep, float value)
{
    const int index = getIndexStepped(phiStep, thetaStep, m_phiSteps);

    m_map[index] += value;
}

void PhiThetaPDF::splat(float phi, float theta, float value)
{
    const int index = getIndex(phi, theta, m_phiSteps, m_thetaSteps);

    m_map[index] += value;
}

void PhiThetaPDF::build()
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

float PhiThetaPDF::eval(float phi, float theta) const
{
    assert(m_built);

    const int index = getIndex(phi, theta, m_phiSteps, m_thetaSteps);
    if (index > 0) {
        return m_cdf[index] - m_cdf[index] - 1;
    } else {
        return m_cdf[index];
    }
}

void PhiThetaPDF::sample(RandomGenerator &random, float *phi, float *theta, float *pdf) const
{
    assert(m_built);

    const float xi = random.next();

    const int length = m_map.size();
    for (int i = 0; i < i < length; i++) {
        if (xi > m_cdf[i]) {
            continue;
        }

        int phiStep, thetaStep;
        getStepsFromIndex(i, &phiStep, &thetaStep, m_phiSteps);

        float phi1 = M_TWO_PI * (1.f * phiStep / m_phiSteps);
        float phi2 = M_TWO_PI * (1.f * (phiStep + 1) / m_phiSteps);

        float theta1 = (M_PI / 2.f) * (1.f * thetaStep / m_thetaSteps);
        float theta2 = (M_PI / 2.f) * (1.f * (thetaStep + 1) / m_thetaSteps);

        assert(phiStep != -1);
        assert(thetaStep != -1);

        const float xiPhi = random.next();
        *phi = lerp(phi1, phi2, xiPhi);

        const float xiY = random.next();
        const float y1 = cosf(theta1);
        const float y2 = cosf(theta2);
        const float ySample = lerp(y1, y2, xiY);
        *theta = acosf(ySample);

        float massRatio = m_cdf[i];
        if (i > 0) {
            massRatio -= m_cdf[i - 1];
        }

        *pdf = massRatio / ((y1 - y2) * (phi2 - phi1));
        return;
    }

    assert(0);
}

void PhiThetaPDF::save() const
{
    Image image(m_phiSteps, m_thetaSteps);
    for (int thetaStep = 0; thetaStep < m_thetaSteps; thetaStep++) {
        for (int phiStep = 0; phiStep < m_phiSteps; phiStep++) {
            int index = getIndexStepped(phiStep, thetaStep, m_phiSteps);

            float value = m_map[index];
            image.set(thetaStep, phiStep, value, value, value);
        }
    }

    image.save("pdf_theta");
}
