#include "photon_pdf.h"

#include "util.h"

#include <assert.h>
#include <math.h>

static const int phiSteps = 20;
static const int thetaSteps = 20;

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
    float massLookup[phiSteps][thetaSteps];
    float totalMass = 0;

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
        const float theta = acosf(wi.y());

        // assert(0.f <= phi);
        // assert(phi <= M_TWO_PI);

        // assert(0.f <= theta);
        // assert(theta <= M_PI);

        const int phiStep = (int)floorf(phi / (M_TWO_PI / phiSteps));
        const int thetaStep = (int)floorf(theta / (M_PI / 2.f / thetaSteps));

        const Color &throughput = point.throughput;
        float mass = throughput.r() + throughput.g() + throughput.b();
        massLookup[phiStep][thetaStep] += mass;
        totalMass += mass;
    }

    // if (debug) {
    //     for (int phi = 0; phi < phiSteps; phi++) {
    //         for (int theta = 0; theta < thetaSteps; theta++) {
    //             printf("%f ", massLookup[phi][theta]);
    //         }
    //         printf("\n");
    //     }
    // }

    float CDF[phiSteps * thetaSteps];
    for (int i = 0; i < phiSteps * thetaSteps; i++) {
        const int phiStep = (int)floorf(i / thetaSteps);
        const int thetaStep = i % thetaSteps;
        CDF[i] = massLookup[phiStep][thetaStep] / totalMass;
        if (i > 0) {
            CDF[i] += CDF[i - 1];
        }
    }

    const float xi = random.next();
    int phiStep = -1, thetaStep = -1;
    for (int i = 0; i < phiSteps * thetaSteps; i++) {
        if (xi <= CDF[i]) {
            phiStep = (int)floorf(i / thetaSteps);
            thetaStep = i % thetaSteps;
            float massRatio = massLookup[phiStep][thetaStep] / totalMass;
            *pdf = massRatio * phiSteps * thetaSteps * INV_TWO_PI;
            break;
        }
    }

    assert(phiStep != -1);
    assert(thetaStep != -1);

    const float xiPhi = random.next();
    const float phiSample = ((1.f - xiPhi) * phiStep + xiPhi * (phiStep + 1)) / phiSteps;

    const float xiTheta = random.next();
    const float thetaSample = ((1.f - xiTheta) * thetaStep + xiTheta * (thetaStep + 1)) / thetaSteps;

    const float phi = M_TWO_PI * phiSample;
    const float theta = (M_PI / 2.f) * thetaSample;

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
