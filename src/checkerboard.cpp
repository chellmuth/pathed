#include "checkerboard.h"

#include <math.h>

Color Checkerboard::lookup(UV uv) const {
    int uColumn = (int)floorf(uv.u * 40);
    int vColumn = (int)floorf(uv.v * 160);
    printf("%i %i %f %f\n", uColumn, vColumn, uv.u, uv.v);
    if (uColumn % 2 == vColumn % 2) {
        return Color(1.f);
    } else {
        return Color(0.f);
    }
}

