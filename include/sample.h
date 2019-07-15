#pragma once

struct Sample {
    std::vector<Point3> eyePoints;
    std::vector<Point3> shadowPoints;

    Sample()
    : eyePoints(), shadowPoints()
    {}
};
