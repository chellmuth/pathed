#include "curve.h"

#include "globals.h"

#include <embree3/rtcore.h>

Curve::Curve(Point3 p1, Point3 p2, Point3 p3, Point3 p4, float width0, float width1)
    : m_p1(p1),
      m_p2(p2),
      m_p3(p3),
      m_p4(p4),
      m_width0(width0),
      m_width1(width1)
{}
