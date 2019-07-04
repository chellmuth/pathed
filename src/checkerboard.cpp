#include "checkerboard.h"

#include <math.h>

Color Checkerboard::lookup(UV uv) const {
    if ((int)floorf(uv.u * 10) % 2 == 0) {
        return Color(1.f);
    }
    return Color(0.f);
}

