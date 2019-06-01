#include "app_controller.h"

#include <iostream>

#include "camera.h"
#include "integrator.h"
#include "intersection.h"
#include "random_generator.h"
#include "ray.h"

AppController::AppController(Scene &scene, int width, int height)
    : mScene(scene),
      mWidth(width),
      mHeight(height),
      mSelectedPoint(0.f, 0.f, 0.f),
      mSample({ Point3(0.f, 0.f, 0.f) })
{}

void AppController::handlePathTraceClick(int x, int y)
{
    int row = (mHeight - 1) - y;
    int col = x;

    Ray ray = mScene.getCamera()->generateRay(
        row, col,
        mWidth, mHeight
    );

    Intersection intersection = mScene.testIntersect(ray);
    if (!intersection.hit) { return; }

    mSelectedPoint = intersection.point;

    Integrator integrator;
    RandomGenerator random;
    int bounceCount = 0;
    std::vector<Vector3> intersectionList;
    Sample sample { ray.origin() };

    Color color = integrator.L(
        intersection,
        mScene,
        random,
        bounceCount,
        intersectionList,
        sample
    );
    mIntersections = intersectionList;
    mSample = sample;
}
