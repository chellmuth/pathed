#pragma once

#include "color.h"
#include "uv.h"

class Albedo {
public:
    virtual Color lookup(UV uv) const = 0;
};
