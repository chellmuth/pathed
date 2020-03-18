#pragma once

#include "color.h"
#include "medium.h"
#include "point.h"
#include "random_generator.h"
#include "vector.h"

#include <assert.h>

#include <cmath>

class HomogeneousAbsorptionMedium : public Medium {
public:
    HomogeneousAbsorptionMedium(Color sigmaA) : m_sigmaA(sigmaA) {};

    Color transmittance(const Point3 &pointA, const Point3 &pointB) const override;

    TransmittanceQueryResult findTransmittance(
        const Point3 &entryPointWorld,
        const Point3 &exitPointWorld,
        float targetTransmittance
    ) const override {
        throw std::runtime_error("Don't call me");
        return TransmittanceQueryResult({ false, -1.f });
    }

    IntegrationResult integrate(
        const Point3 &entryPointWorld,
        const Point3 &exitPointWorld,
        const Scene &scene,
        RandomGenerator &random
    ) const override;

    float sigmaT(const Point3 &worldPoint) const override {
        throw std::runtime_error("Don't call me");
        return -1.f;
    }

    float sigmaS(const Point3 &worldPoint) const override {
        throw std::runtime_error("Don't call me");
        return -1.f;
    }

private:
    Color m_sigmaA;
};
