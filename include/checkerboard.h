#pragma once

#include "albedo.h"
#include "color.h"
#include "uv.h"

class Checkerboard : public Albedo {
    Color lookup(UV uv) const override;
};
