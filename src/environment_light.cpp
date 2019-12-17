#include "environment_light.h"

#include "coordinate.h"
#include "measure.h"
#include "monte_carlo.h"
#include "util.h"

#include "tinyexr.h"

#include <assert.h>
#include <algorithm>
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

    std::vector<float> data(m_width * m_height);
    for (int i = 0; i < m_width * m_height; i++) {
        data[i] += m_data[4 * i + 0];
        data[i] += m_data[4 * i + 1];
        data[i] += m_data[4 * i + 2];
    }

    std::vector<float> thetaData(m_height, 0.f);
    for (int thetaStep = 0; thetaStep < m_height; thetaStep++) {
        float thetaSum = 0.f;
        std::vector<float> phiData(m_width, 0.f);
        for (int phiStep = 0; phiStep < m_width; phiStep++) {
            int index = thetaStep * m_width + phiStep;
            float value = data[index];

            thetaSum += value;
            phiData[phiStep] = value;
        }

        m_phiDistributions.push_back(Distribution(phiData));
        thetaData[thetaStep] = thetaSum;
    }

    m_thetaDistribution.reset(new Distribution(thetaData));
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

    const float phiCanonical = util::clampClose(phi / M_TWO_PI, 0.f, 1.f);
    const float thetaCanonical = util::clampClose(theta / M_PI, 0.f, 1.f);

    const int phiStep = std::min((int)floorf(m_width * phiCanonical), m_width - 1);
    const int thetaStep = std::min((int)floorf(m_height * thetaCanonical), m_height - 1);

    const int index = thetaStep * m_width + phiStep;
    Color result(m_data[4*index + 0], m_data[4*index + 1], m_data[4*index + 2]);

    return result * m_scale;
}

SurfaceSample EnvironmentLight::sample(const Intersection &intersection, RandomGenerator &random) const
{
    float thetaPDF, phiPDF;
    const int thetaStep = m_thetaDistribution->sample(&thetaPDF, random);
    const int phiStep = m_phiDistributions[thetaStep].sample(&phiPDF, random);

    const float phiCanonical = (phiStep + 0.5f) / m_width;
    const float thetaCanonical = (thetaStep + 0.5f) / m_height;

    const float phi = phiCanonical * M_TWO_PI;
    const float theta = thetaCanonical * M_PI;

    const float pdf = thetaPDF * phiPDF * m_width * m_height / (sinf(theta) * M_TWO_PI * M_PI);

    const Vector3 directionLocal = sphericalToCartesian(phi, theta);
    const Vector3 direction(directionLocal.x(), directionLocal.z(), directionLocal.y());

    SurfaceSample inProgress = {
        .point = intersection.point + direction * 10000.f,
        .normal = direction * -1.f,
        .invPDF = 1.f / pdf,
        .measure = Measure::SolidAngle
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

float EnvironmentLight::emitPDF(const Vector3 &direction) const
{
    float phi, theta;
    cartesianToSpherical(direction, &phi, &theta);

    const float phiCanonical = phi / M_TWO_PI;
    const float thetaCanonical = theta / M_PI;

    const int phiStep = std::min((int)floorf(phiCanonical * m_width), m_width - 1);
    const int thetaStep = std::min((int)floorf(thetaCanonical * m_height), m_height - 1);

    const float thetaPDF = m_thetaDistribution->pdf(thetaStep);
    const float phiPDF = m_phiDistributions[thetaStep].pdf(phiStep);

    return thetaPDF * phiPDF;
}

Color EnvironmentLight::biradiance(const SurfaceSample &lightSample, const Point3 &surfacePoint) const
{
    Vector3 direction = (lightSample.point - surfacePoint).toVector();

    return emit(direction.normalized());
}
