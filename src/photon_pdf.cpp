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
    : mOrigin(origin),
      mDataSource(dataSource),
      mIndices(indices)
{}

Vector3 PhotonPDF::sample(RandomGenerator &random, const Transform &worldToNormal, float *pdf, bool debug)
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

    for (size_t index : *mIndices) {
        const DataSource::Point &point = mDataSource->points[index];

        const Point3 &source = point.source;
        const Vector3 wi = worldToNormal.apply(
            (source - mOrigin).toVector()
        ).normalized();

        float phi = atan2f(wi.z(), wi.x());
        if (phi < 0.f) {
            phi += 2 * M_PI;
        }
        if (phi == M_TWO_PI) {
            phi = 0;
        }

        const float theta = acosf(wi.y());
        if (theta > M_PI / 2.f) { continue; }

        // assert(0.f <= phi);
        // assert(phi <= M_TWO_PI);

        // assert(0.f <= theta);
        // assert(theta <= M_PI);

        const int phiStep = (int)floorf(phi / (M_TWO_PI / phiSteps));
        const int thetaStep = (int)floorf(theta / (M_PI / 2.f / thetaSteps));

        assert(phiStep <= phiSteps);
        assert(thetaSteps <= thetaSteps);

        const Color &throughput = point.throughput;
        float mass = throughput.r() + throughput.g() + throughput.b();
        massLookup[phiStep][thetaStep] += mass;
        totalMass += mass;
    }

    // const float addedMass = totalMass * 0.3f;
    // const float addedMassPerCell = addedMass / (thetaSteps * phiSteps);
    // for (int phi = 0; phi < phiSteps; phi++) {
    //     for (int theta = 0; theta < thetaSteps; theta++) {
    //         massLookup[phi][theta] += addedMassPerCell;
    //     }
    // }
    // totalMass += addedMass;

    if (totalMass == 0) {
        // printf("0 Mass!\n");
        *pdf = INV_TWO_PI;
        return UniformSampleHemisphere(random);
    }

    float CDF[phiSteps * thetaSteps];
    for (int i = 0; i < phiSteps * thetaSteps; i++) {
        const int phiStep = (int)floorf(i / thetaSteps);
        const int thetaStep = i % thetaSteps;
        CDF[i] = massLookup[phiStep][thetaStep] / totalMass;
        if (i > 0) {
            CDF[i] += CDF[i - 1];
        }
    }

    assert(fabsf(CDF[phiSteps * thetaSteps - 1] - 1.f) < 1e-5);

    const float xi = random.next();
    int phiStep = -1, thetaStep = -1;
    for (int i = 0; i < phiSteps * thetaSteps; i++) {
        if (xi <= CDF[i]) {
            phiStep = (int)floorf(i / thetaSteps);
            thetaStep = i % thetaSteps;
            float massRatio = massLookup[phiStep][thetaStep] / totalMass;

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

            // const float xiTheta = random.next();
            // const float thetaSample = ((1.f - xiTheta) * thetaStep + xiTheta * (thetaStep + 1)) / thetaSteps;

            const float phi = M_TWO_PI * phiSample;
            // const float theta = (M_PI / 2.f) * thetaSample;

            // *pdf = massRatio * sinf(theta) / ((cosf(theta1) - cosf(theta2)) * (phi2 - phi1));
            // *pdf = massRatio / ((cosf(theta1) - cosf(theta2)) * (phi2 - phi1));
            *pdf = massRatio / ((y1 - y2) * (phi2 - phi1));

            const float y = cosf(theta);
            const float x = sinf(theta) * cosf(phi);
            const float z = sinf(theta) * sinf(phi);

            Vector3 result = Vector3(x, y, z);
            if (debug) {
                printf("steps: %i %i %f\n", phiStep, thetaStep, massLookup[phiStep][thetaStep]);
                printf("phi/theta: %f %f\n", phi, theta);
                result.debug();
            }
            assert(fabsf(result.length() - 1.f) < 1e-5);

            return result;
        }
    }
}
