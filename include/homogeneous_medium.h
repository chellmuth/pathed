#pragma once

#include "color.h"
#include "medium.h"
#include "point.h"
#include "util.h"
#include "vector.h"

class HomogeneousMedium : public Medium {
public:
    HomogeneousMedium(Color sigmaT) : m_sigmaT(sigmaT), Medium() {}

    Color transmittance(const Point3 &pointA, const Point3 &pointB) const override {
        const Vector3 path = (pointB - pointA).toVector();
        return util::exp(-m_sigmaT * path.length());
    }

private:
    Color m_sigmaT;
};
