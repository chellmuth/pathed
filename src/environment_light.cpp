#include "environment_light.h"

#include "coordinate.h"
#include "monte_carlo.h"
#include "util.h"

#include "tinyexr.h"

#include <assert.h>
#include <cmath>

EnvironmentLight::EnvironmentLight(std::string filename, float scale)
    : Light(), m_filename(filename), m_scale(scale)
{
    const char *error = nullptr;

    int code = LoadEXR(&m_data, &m_width, &m_height, filename.c_str(), &error);
    if (code != TINYEXR_SUCCESS) {
        fprintf(stderr, "ERROR: %s\n", error);
        FreeEXRErrorMessage(error);
    }
}

Color EnvironmentLight::emit() const
{
    return Color(0.f, 20.f, 0.f);
}

Color EnvironmentLight::emit(const Vector3 &direction) const
{
    float phi, theta;
    cartesianToSpherical(
        Vector3(direction.x(), direction.z(), direction.y()),
        &phi, &theta
    );

    const float phiCanonical = phi / M_TWO_PI;
    const float thetaCanonical = theta / M_PI;

    assert(0.f <= phiCanonical && phiCanonical <= 1.f);
    assert(0.f <= thetaCanonical && thetaCanonical <= 1.f);

    int phiStep = (int)floorf(m_width * phiCanonical);
    int thetaStep = (int)floorf(m_height * thetaCanonical);

    int index = thetaStep * m_width + phiStep;
    Color result(m_data[4*index + 0], m_data[4*index + 1], m_data[4*index + 2]);

    return result * m_scale;
}

SurfaceSample EnvironmentLight::sample(const Intersection &intersection, RandomGenerator &random) const
{
    Vector3 direction = UniformSampleSphere(random);

    SurfaceSample inProgress = {
        .point = intersection.point + (direction * 10000.f),
        .normal = direction * -1.f,
        .invPDF = 1.f / UniformSampleSpherePDF(direction)
    };
    return inProgress;
}

SurfaceSample EnvironmentLight::sampleEmit(RandomGenerator &random) const
{
    SurfaceSample fake = {
        .point = Point3(0.f, 0.f, 0.f),
        .normal = Vector3(0.f),
        .invPDF = 0.f
    };
    return fake;
}

Color EnvironmentLight::biradiance(const SurfaceSample &lightSample, const Point3 &surfacePoint) const
{
    Vector3 direction = (lightSample.point - surfacePoint).toVector();

    return emit(direction.normalized());
}
