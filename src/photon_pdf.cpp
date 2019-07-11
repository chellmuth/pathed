#include "photon_pdf.h"

#include "globals.h"
#include "job.h"
#include "monte_carlo.h"
#include "util.h"

#include <assert.h>
#include <math.h>

PhotonPDF::PhotonPDF(
    const Point3 &origin,
    std::shared_ptr<DataSource> dataSource,
    std::shared_ptr<std::vector<size_t> > indices
)
    : m_origin(origin),
      m_dataSource(dataSource),
      m_indices(indices),
      m_emptyCDF(false)
{}

void cartesianToSpherical(Vector3 cartesian, float *phi, float *theta)
{
    *phi = atan2f(cartesian.z(), cartesian.x());
    if (*phi < 0.f) {
        *phi += 2 * M_PI;
    }
    if (*phi == M_TWO_PI) {
        *phi = 0;
    }

    *theta = acosf(cartesian.y());
}

void PhotonPDF::buildCDF(const Transform &worldToNormal)
{
    const int phiSteps = g_job->phiSteps();
    const int thetaSteps = g_job->thetaSteps();

    float massLookup[phiSteps][thetaSteps];
    float totalMass = 0.f;

    for (int phi = 0; phi < phiSteps; phi++) {
        for (int theta = 0; theta < thetaSteps; theta++) {
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

        const int phiStep = (int)floorf(phi / (M_TWO_PI / phiSteps));
        const int thetaStep = (int)floorf(theta / (M_PI / 2.f / thetaSteps));

        assert(phiStep < phiSteps);
        assert(thetaStep < thetaSteps);

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

    // int emptyCells = 0;
    // for (int phi = 0; phi < phiSteps; phi++) {
    //     for (int theta = 0; theta < thetaSteps; theta++) {
    //         if (massLookup[phi][theta] == 0.f) {
    //             emptyCells += 1;
    //         }
    //     }
    // }

    // if (emptyCells > 0) {
    //     const float addedMass = totalMass * 0.2f;
    //     const float addedMassPerCell = addedMass / (emptyCells);

    //     for (int phi = 0; phi < phiSteps; phi++) {
    //         for (int theta = 0; theta < thetaSteps; theta++) {
    //             if (massLookup[phi][theta] == 0.f) {
    //                 massLookup[phi][theta] = addedMassPerCell;
    //             }
    //         }
    //     }

    //     totalMass += addedMass;
    // }

    m_CDF.reserve(phiSteps * thetaSteps);
    for (int i = 0; i < phiSteps * thetaSteps; i++) {
        const int phiStep = (int)floorf(i / thetaSteps);
        const int thetaStep = i % thetaSteps;
        m_CDF[i] = massLookup[phiStep][thetaStep] / totalMass;
        if (i > 0) {
            m_CDF[i] += m_CDF[i - 1];
        }
    }

    assert(fabsf(m_CDF[phiSteps * thetaSteps - 1] - 1.f) < 1e-5);
}

Vector3 PhotonPDF::sample(RandomGenerator &random, const Transform &worldToNormal, float *pdf, bool debug)
{
    buildCDF(worldToNormal);

    if (m_emptyCDF) {
        *pdf = INV_TWO_PI;
        return UniformSampleHemisphere(random);
    }

    const float xi = random.next();

    const int phiSteps = g_job->phiSteps();
    const int thetaSteps = g_job->thetaSteps();
    int phiStep = -1, thetaStep = -1;

    for (int i = 0; i < phiSteps * thetaSteps; i++) {
        if (xi <= m_CDF[i]) {
            phiStep = (int)floorf(i / thetaSteps);
            thetaStep = i % thetaSteps;

            float massRatio = m_CDF[i];
            if (i > 0) {
                massRatio -= m_CDF[i - 1];
            }

            float phi1 = M_TWO_PI * (1.f * phiStep / phiSteps);
            float phi2 = M_TWO_PI * (1.f * (phiStep + 1) / phiSteps);

            float theta1 = (M_PI / 2.f) * (1.f * thetaStep / thetaSteps);
            float theta2 = (M_PI / 2.f) * (1.f * (thetaStep + 1) / thetaSteps);

            assert(phiStep != -1);
            assert(thetaStep != -1);

            const float xiPhi = random.next();
            const float phiSample = ((1.f - xiPhi) * phiStep + xiPhi * (phiStep + 1)) / phiSteps;

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
    assert(theta <= M_PI);

    const int phiSteps = g_job->phiSteps();
    const int thetaSteps = g_job->thetaSteps();

    const int phiStep = (int)floorf(phi / (M_TWO_PI / phiSteps));
    const int thetaStep = (int)floorf(theta / (M_PI / 2.f / thetaSteps));

    assert(phiStep < phiSteps);
    assert(thetaStep < thetaSteps);

    const float phi1 = M_TWO_PI * (1.f * phiStep / phiSteps);
    const float phi2 = M_TWO_PI * (1.f * (phiStep + 1) / phiSteps);

    const float theta1 = (M_PI / 2.f) * (1.f * thetaStep / thetaSteps);
    const float theta2 = (M_PI / 2.f) * (1.f * (thetaStep + 1) / thetaSteps);

    const float y1 = cosf(theta1);
    const float y2 = cosf(theta2);


    const int cdfIndex = phiStep * thetaSteps + thetaStep;
    float massRatio = m_CDF[cdfIndex];
    if (cdfIndex > 0) {
        massRatio -= m_CDF[cdfIndex - 1];
    }
    const float pdf = massRatio / ((y1 - y2) * (phi2 - phi1));
    return pdf;
}
