#pragma once

#include "color.h"
#include "medium.h"
#include "point.h"
#include "util.h"
#include "vector.h"

#include <cmath>

class HomogeneousMedium : public Medium {
public:
    HomogeneousMedium(Color sigmaT) : m_sigmaT(sigmaT), Medium() {}

    Color transmittance(const Point3 &pointA, const Point3 &pointB) const override {
        const Vector3 path = (pointB - pointA).toVector();
        return util::exp(-m_sigmaT * path.length());
    }

    TransmittanceQueryResult findTransmittance(
        const Point3 &entryPointWorld,
        const Point3 &exitPointWorld,
        float targetTransmittance
    ) const override {
        assert(m_sigmaT.r() == m_sigmaT.g() && m_sigmaT.r() == m_sigmaT.b());

        const float distance = -m_sigmaT.r() / std::log(targetTransmittance);
        const float maxDistance = (exitPointWorld - entryPointWorld).toVector().length();
        if (distance > maxDistance) {
            return TransmittanceQueryResult({ false, -1.f });
        } else {
            return TransmittanceQueryResult({ true, distance });
        }
    }
private:
    Color m_sigmaT;
};
