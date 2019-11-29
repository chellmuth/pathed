#pragma once

#include "color.h"
#include "intersection.h"
#include "light.h"
#include "material.h"
#include "point.h"
#include "random_generator.h"
#include "surface.h"

#include <iostream>
#include <sstream>
#include <string>

class EnvironmentLight : public Light {
public:
    EnvironmentLight(std::string filename);

    Color emit() const override;
    Color emit(const Vector3 &direction) const;

    SurfaceSample sample(
        const Intersection &intersection,
        RandomGenerator &random
    ) const override;

    SurfaceSample sampleEmit(RandomGenerator &random) const override;

    Color biradiance(
        const SurfaceSample &lightSample,
        const Point3 &surfacePoint
    ) const override;

    std::string toString() const {
        std::ostringstream oss;
        oss << "[EnvironmentLight file=" << m_filename << "]";
        return oss.str();
    }

private:
    std::string m_filename;
    float *m_data;
    int m_width, m_height;
};
