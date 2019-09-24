#include "photon_pdf.h"

#include "coordinate.h"
#include "monte_carlo.h"
#include "util.h"

#include <assert.h>
#include <math.h>

PhotonPDF::PhotonPDF(
    const Point3 &origin,
    std::shared_ptr<DataSource> dataSource,
    std::shared_ptr<std::vector<size_t> > indices,
    int phiSteps,
    int thetaSteps

)
    : m_origin(origin),
      m_dataSource(dataSource),
      m_indices(indices),
      m_phiSteps(phiSteps),
      m_thetaSteps(thetaSteps),
      m_emptyCDF(false)
{}

void PhotonPDF::buildCDF(const Transform &worldToNormal)
{
    float massLookup[m_phiSteps][m_thetaSteps];
    float totalMass = 0.f;

    for (int phi = 0; phi < m_phiSteps; phi++) {
        for (int theta = 0; theta < m_thetaSteps; theta++) {
            massLookup[phi][theta] = 0.f;
        }
    }

    for (size_t index : *m_indices) {
        const DataSource::Point &point = m_dataSource->points[index];

        const Point3 &source = point.source;
        const Vector3 wi = worldToNormal.apply(
            (source - m_origin).toVector()
        ).normalized();

        float phi, theta;
        cartesianToSpherical(wi, &phi, &theta);

        if (theta > M_PI / 2.f) { continue; }

        const int phiStep = (int)floorf(phi / (M_TWO_PI / m_phiSteps));
        const int thetaStep = (int)floorf(theta / (M_PI / 2.f / m_thetaSteps));

        assert(phiStep < m_phiSteps);
        assert(thetaStep < m_thetaSteps);

        const Color &throughput = point.throughput;
        float mass = throughput.r() + throughput.g() + throughput.b();
        massLookup[phiStep][thetaStep] += mass;
        totalMass += mass;
    }

    if (totalMass == 0.f) {
        m_emptyCDF = true;
        // printf("0 Mass!\n");
        return;
    }

    // const float addedMass = totalMass * 0.2f;
    // const float addedMassPerCell = addedMass / (m_phiSteps * m_thetaSteps);

    // for (int phi = 0; phi < m_phiSteps; phi++) {
    //     for (int theta = 0; theta < m_thetaSteps; theta++) {
    //         massLookup[phi][theta] += addedMassPerCell;
    //     }
    // }

    // totalMass += addedMass;

    m_CDF.reserve(m_phiSteps * m_thetaSteps);
    for (int i = 0; i < m_phiSteps * m_thetaSteps; i++) {
        const int phiStep = (int)floorf(i / m_thetaSteps);
        const int thetaStep = i % m_thetaSteps;
        m_CDF[i] = massLookup[phiStep][thetaStep] / totalMass;
        if (i > 0) {
            m_CDF[i] += m_CDF[i - 1];
        }
    }

    assert(fabsf(m_CDF[m_phiSteps * m_thetaSteps - 1] - 1.f) < 1e-5);
}

Vector3 PhotonPDF::sample(RandomGenerator &random, const Transform &worldToNormal, float *pdf, bool debug)
{
    buildCDF(worldToNormal);

    if (m_emptyCDF) {
        *pdf = INV_TWO_PI;
        return UniformSampleHemisphere(random);
    }

    const float xi = random.next();

    int phiStep = -1, thetaStep = -1;

    for (int i = 0; i < m_phiSteps * m_thetaSteps; i++) {
        if (xi <= m_CDF[i]) {
            phiStep = (int)floorf(i / m_thetaSteps);
            thetaStep = i % m_thetaSteps;

            float massRatio = m_CDF[i];
            if (i > 0) {
                massRatio -= m_CDF[i - 1];
            }

            float phi1 = M_TWO_PI * (1.f * phiStep / m_phiSteps);
            float phi2 = M_TWO_PI * (1.f * (phiStep + 1) / m_phiSteps);

            float theta1 = (M_PI / 2.f) * (1.f * thetaStep / m_thetaSteps);
            float theta2 = (M_PI / 2.f) * (1.f * (thetaStep + 1) / m_thetaSteps);

            assert(phiStep != -1);
            assert(thetaStep != -1);

            const float xiPhi = random.next();
            const float phiSample = ((1.f - xiPhi) * phiStep + xiPhi * (phiStep + 1)) / m_phiSteps;

            const float xiY = random.next();
            const float y1 = cosf(theta1);
            const float y2 = cosf(theta2);
            const float ySample = (1.f - xiY) * y1 + xiY * y2;
            const float theta = acosf(ySample);

            *pdf = massRatio / ((y1 - y2) * (phi2 - phi1));

            const float phi = M_TWO_PI * phiSample;
            const float y = cosf(theta);
            const float x = sinf(theta) * cosf(phi);
            const float z = sinf(theta) * sinf(phi);

            Vector3 result = Vector3(x, y, z);
            assert(fabsf(result.length() - 1.f) < 1e-5);

            return result;
        }
    }

    assert(false);
    return Vector3(0.f, 0.f, 0.f);
}

float PhotonPDF::pdf(const Vector3 &wiWorld, const Transform &worldToNormal)
{
    if (m_emptyCDF) {
        return INV_TWO_PI;
    }

    Vector3 wi(worldToNormal.apply(wiWorld));

    float phi, theta;
    cartesianToSpherical(wi, &phi, &theta);

    assert(0.f <= phi);
    assert(phi <= M_TWO_PI);

    assert(0.f <= theta);
    assert(theta < M_PI/2.f);

    const int phiStep = (int)floorf(phi / (M_TWO_PI / m_phiSteps));
    const int thetaStep = (int)floorf(theta / (M_PI / 2.f / m_thetaSteps));

    assert(phiStep < m_phiSteps);
    assert(thetaStep < m_thetaSteps);

    const float phi1 = M_TWO_PI * (1.f * phiStep / m_phiSteps);
    const float phi2 = M_TWO_PI * (1.f * (phiStep + 1) / m_phiSteps);

    const float theta1 = (M_PI / 2.f) * (1.f * thetaStep / m_thetaSteps);
    const float theta2 = (M_PI / 2.f) * (1.f * (thetaStep + 1) / m_thetaSteps);

    const float y1 = cosf(theta1);
    const float y2 = cosf(theta2);

    const int cdfIndex = phiStep * m_thetaSteps + thetaStep;
    float massRatio = m_CDF[cdfIndex];
    if (cdfIndex > 0) {
        massRatio -= m_CDF[cdfIndex - 1];
    }
    const float pdf = massRatio / ((y1 - y2) * (phi2 - phi1));
    assert(pdf != 0.f);
    return pdf;
}

std::vector<float> PhotonPDF::asVector(const Transform &worldToNormal)
{
    buildCDF(worldToNormal);

    std::vector<float> result(m_thetaSteps * m_phiSteps);

    for (int thetaStep = 0; thetaStep < m_thetaSteps; thetaStep++) {
        for (int phiStep = 0; phiStep < m_phiSteps; phiStep++) {
            int index = phiStep * m_thetaSteps + thetaStep;

            float pdf = m_CDF[index];
            if (index > 0) {
                pdf -= m_CDF[index - 1];
            }

            result[index] = pdf;
        }
    }

    return result;
}

void PhotonPDF::save(const std::string &filename, const Transform &worldToNormal)
{
    buildCDF(worldToNormal);

    Image image(m_phiSteps, m_thetaSteps);
    for (int thetaStep = 0; thetaStep < m_thetaSteps; thetaStep++) {
        for (int phiStep = 0; phiStep < m_phiSteps; phiStep++) {
            int index = phiStep * m_thetaSteps + thetaStep;

            float pdf = m_CDF[index];
            if (index > 0) {
                pdf -= m_CDF[index - 1];
            }

            image.set(m_thetaSteps - thetaStep - 1, phiStep, pdf, pdf, pdf);
        }
    }

    image.write(filename);
}
