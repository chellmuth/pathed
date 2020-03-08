#pragma once

#include "albedo.h"
#include "color.h"
#include "intersection.h"

class Checkerboard : public Albedo {
public:
    Checkerboard(Color onColor, Color offColor, UV resolution);

    Color lookup(const Intersection &intersection) const override;

private:
    Color m_onColor;
    Color m_offColor;
    UV m_resolution;
};
