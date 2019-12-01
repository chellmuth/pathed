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

    m_cdf.resize(m_width * m_height, 0.f);
    float sum = 0.f;
    for (int i = 0; i < m_width * m_height; i++) {
        m_cdf[i] += m_data[4 * i + 0];
        m_cdf[i] += m_data[4 * i + 1];
        m_cdf[i] += m_data[4 * i + 2];

        if (i > 0) {
            m_cdf[i] += m_cdf[i - 1];
        }

        sum += m_data[4 * i + 0];
        sum += m_data[4 * i + 1];
        sum += m_data[4 * i + 2];
    }

    for (int i = 0; i < m_width * m_height; i++) {
        m_cdf[i] /= sum;
    }

    assert(fabs(m_cdf[m_width * m_height - 1] - 1.f) < 1e-5);
    m_cdf[m_width * m_height - 1] = 1.f;
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
    float xi = random.next();

    const int cdfIndex = binarySearchCDF(m_cdf, xi);
    const int phiStep = cdfIndex % m_width;
    const int thetaStep = (int)floorf(cdfIndex / m_width);

    assert(xi <= m_cdf[cdfIndex]);
    if (cdfIndex > 0) {
        assert(xi > m_cdf[cdfIndex - 1]);
    }

    float pdf = m_cdf[cdfIndex];
    if (cdfIndex > 0) {
        pdf -= m_cdf[cdfIndex - 1];
    }

    const float phiCanonical = (phiStep + 0.5f) / m_width;
    const float thetaCanonical = (thetaStep + 0.5f) / m_height;

    const float phi = phiCanonical * M_TWO_PI;
    const float theta = thetaCanonical * M_PI;

    pdf = pdf * m_width * m_height / (sinf(theta) * M_TWO_PI * M_PI);

    const Vector3 directionLocal = sphericalToCartesian(phi, theta);
    const Vector3 direction(directionLocal.x(), directionLocal.z(), directionLocal.y());

    SurfaceSample inProgress = {
        .point = intersection.point + direction * 10000.f,
        .normal = direction * -1.f,
        .invPDF = 1.f / pdf
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
