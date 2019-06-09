#include "app_controller.h"

#include <iostream>

#include "bdpt.h"
#include "camera.h"
#include "path_tracer.h"
#include "intersection.h"
#include "random_generator.h"
#include "ray.h"

AppController::AppController(Scene &scene, int width, int height)
    : mScene(scene),
      mWidth(width),
      mHeight(height),
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

    BDPT integrator;
    RandomGenerator random;
    int bounceCount = 10;
    Sample sample { ray.origin() };

    Color color = integrator.L(
        intersection,
        mScene,
        random,
        bounceCount,
        sample
    );
    mSample = sample;
}
