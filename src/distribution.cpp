#include "distribution.h"

#include <assert.h>
#include <cmath>

Distribution::Distribution(const std::vector<float> &values)
{
    const size_t size = values.size();

    m_cdf.resize(size, 0.f);

    float sum = 0.f;
    for (int i = 0; i < size; i++) {
        sum += values[i];
    }

    for (int i = 0; i < size; i++) {
        m_cdf[i] = values[i] / sum;

        if (i > 0) {
            m_cdf[i] += m_cdf[i - 1];
        }
    }

    assert(fabs(m_cdf[size - 1] - 1.f) < 1e-5);
    m_cdf[size - 1] = 1.f;
}

int Distribution::sample(float *pdf, RandomGenerator &random) const
{
    float xi = random.next();

    for (int i = 0; i < m_cdf.size(); i++) {
        if (xi <= m_cdf[i]) {
            if (i > 0) {
                *pdf = m_cdf[i] - m_cdf[i - 1];
            } else {
                *pdf = m_cdf[i];
            }

            return i;
        }
    }
    assert(false);
}

float Distribution::pdf(int index) const
{
    float cdf = m_cdf[index];
    if (index == 0) {
        return cdf;
    }
    return cdf - m_cdf[index - 1];
}
