#include "photon_pdf.h"

#include "util.h"

#include <assert.h>
#include <math.h>

static const int phiSteps = 50;
static const int thetaSteps = 50;

PhotonPDF::PhotonPDF(
    const Point3 &origin,
    std::shared_ptr<DataSource> dataSource,
    std::shared_ptr<std::vector<size_t> > indices
)
    : mOrigin(origin),
      mDataSource(dataSource),
      mIndices(indices)
{}

Vector3 PhotonPDF::sample(RandomGenerator &random, float *pdf)
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
        const Vector3 wi = (source - mOrigin).toVector().normalized();

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
        const int thetaStep = (int)floorf(theta / (M_PI / thetaSteps));

        const Color &throughput = point.throughput;
        float mass = throughput.r() + throughput.g() + throughput.b();
        massLookup[phiStep][thetaStep] += mass;
        totalMass += mass;
    }

    float CDF[phiSteps * thetaSteps];
    for (int i = 0; i < phiSteps * thetaSteps; i++) {
        const int phiStep = (int)floorf(i / phiSteps);
        const int thetaStep = i % phiSteps;
        CDF[i] = massLookup[phiStep][thetaStep] / totalMass;
        if (i > 0) {
            CDF[i] += CDF[i - 1];
        }
    }

    const float xi = random.next();
    int phiStep = -1, thetaStep = -1;
    for (int i = 0; i < phiSteps * thetaSteps; i++) {
        if (xi <= CDF[i]) {
            phiStep = (int)floorf(i / phiSteps);
            thetaStep = i % phiSteps;
            *pdf = CDF[i];
            break;
        }
    }

    assert(phiStep != -1);
    assert(thetaStep != -1);

    const float xiPhi = random.next();
    const float phiSample = (1.f - xiPhi) * phiStep + xiPhi * (phiStep + 1);

    const float xiTheta = random.next();
    const float thetaSample = (1.f - xiTheta) * thetaStep + xiTheta * (thetaStep + 1);

    const float phi = M_TWO_PI * phiSample;
    const float theta = M_PI * thetaSample;

    const float y = cosf(theta);
    const float x = sinf(theta) * cosf(phi);
    const float z = sinf(theta) * sinf(phi);

    Vector3 result = Vector3(x, y, z);
    assert(fabsf(result.length() - 1.f) < 1e-5);
    return result;
}
