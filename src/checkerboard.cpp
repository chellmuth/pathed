#include "checkerboard.h"

#include <math.h>

Color Checkerboard::lookup(UV uv) const {
    int uIndex = (int)floorf(uv.u * 40);
    int vIndex = (int)floorf(uv.v * 160);

    if (uIndex % 2 == vIndex % 2) {
        return Color(0.f);
    } else {
        return Color(1.f);
    }
}

