#pragma once

#include "color.h"
#include "medium.h"
#include "point.h"
#include "random_generator.h"
#include "vector.h"

#include <assert.h>

#include <cmath>

class HomogeneousMedium : public Medium {
public:
    HomogeneousMedium(Color sigmaT, Color sigmaS);

    Color transmittance(const Point3 &pointA, const Point3 &pointB) const override;

    TransmittanceQueryResult findTransmittance(
        const Point3 &entryPointWorld,
        const Point3 &exitPointWorld,
        float targetTransmittance
    ) const override;

    IntegrationResult integrate(
        const Point3 &entryPointWorld,
        const Point3 &exitPointWorld,
        const Scene &scene,
        RandomGenerator &random
    ) const override;

    float sigmaT(const Point3 &worldPoint) const override {
        assert(m_sigmaT.r() == m_sigmaT.g() && m_sigmaT.r() == m_sigmaT.b());
        return m_sigmaT.r();
    }

    float sigmaS(const Point3 &worldPoint) const override {
        assert(m_sigmaS.r() == m_sigmaS.g() && m_sigmaS.r() == m_sigmaS.b());
        return m_sigmaS.r();
    }

private:
    Color m_sigmaT;
    Color m_sigmaS;
};
