#include "checkerboard.h"

#include <math.h>

Checkerboard::Checkerboard(Color onColor, Color offColor, UV resolution)
    : m_onColor(onColor), m_offColor(offColor), m_resolution(resolution)
{}

Color Checkerboard::lookup(UV uv) const {
    int uIndex = (int)floorf(uv.u * m_resolution.u);
    int vIndex = (int)floorf(uv.v * m_resolution.v);

    if (uIndex % 2 == vIndex % 2) {
        return m_onColor;
    } else {
        return m_offColor;
    }
}

