#pragma once

#include "color.h"
#include "medium.h"
#include "point.h"
#include "vector.h"

#include <assert.h>

#include <cmath>

class HomogeneousMedium : public Medium {
public:
    HomogeneousMedium(Color sigmaT);

    Color transmittance(const Point3 &pointA, const Point3 &pointB) const override;

    TransmittanceQueryResult findTransmittance(
        const Point3 &entryPointWorld,
        const Point3 &exitPointWorld,
        float targetTransmittance
    ) const override;

    Color integrate(
        const Point3 &entryPointWorld,
        const Point3 &exitPointWorld,
        Scene &scene
    ) const override;

    float sigmaT(const Point3 &worldPoint) const override {
        assert(m_sigmaT.r() == m_sigmaT.g() && m_sigmaT.r() == m_sigmaT.b());
        return m_sigmaT.r();
    }

    float sigmaS(const Point3 &worldPoint) const override {
        return sigmaT(worldPoint);
    }

private:
    Color m_sigmaT;
};
