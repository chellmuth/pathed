#pragma once

#include "color.h"
#include "intersection.h"

class Albedo {
public:
    virtual Color lookup(const Intersection &intersection) const = 0;
};
