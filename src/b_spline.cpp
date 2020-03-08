#include "b_spline.h"

#include "globals.h"

#include <embree3/rtcore.h>

#include <iostream>

BSpline::BSpline(std::vector<Point3> points, float width0, float width1)
    : m_points(points),
      m_width0(width0),
      m_width1(width1)
{}
