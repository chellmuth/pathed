#pragma once

#include "color.h"
#include "distribution.h"
#include "light.h"
#include "material.h"
#include "point.h"
#include "random_generator.h"
#include "surface.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

class EnvironmentLight : public Light {
public:
    EnvironmentLight(std::string filename, float scale);

    Color emit() const override;
    Color emit(const Vector3 &direction) const;

    SurfaceSample sample(
        const Point3 &point,
        RandomGenerator &random
    ) const override;

    SurfaceSample sampleEmit(RandomGenerator &random) const override;
    float emitPDF(const Point3 &point, const Vector3 &direction) const override
    {
        return emitPDF(direction);
    }
    float emitPDF(const Vector3 &direction) const;

    Color biradiance(
        const SurfaceSample &lightSample,
        const Point3 &surfacePoint
    ) const override;

    std::string toString() const {
        std::ostringstream oss;
        oss << "[EnvironmentLight scale=" << m_scale << " file=" << m_filename << "]";
        return oss.str();
    }

private:
    float m_scale;

    std::vector<float> m_cdf;

    float *m_data;
    int m_width, m_height;

    std::string m_filename;

    std::unique_ptr<Distribution> m_thetaDistribution;
    std::vector<Distribution> m_phiDistributions;
};
