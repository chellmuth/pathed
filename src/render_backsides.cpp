#include "render_backsides.h"

Color RenderBacksides::L(
    const Intersection &intersection,
    const Scene &scene,
    RandomGenerator &random,
    Sample &sample
) const {
    if (!intersection.hit) { return Color(0.f); }

    if (IntersectionHelper::checkBacksideIntersection(intersection)) {
        std::cout << intersection.normal.toString() << " " << intersection.shadingNormal.toString() << std::endl;
        return Color(1.f, 0.f, 0.f);
    } else {
        return Color(1.f);
    }
}


