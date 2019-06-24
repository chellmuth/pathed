#include "app_controller.h"

#include <ctime>
#include <iostream>

#include "bdpt.h"
#include "camera.h"
#include "depositer.h"
#include "path_tracer.h"
#include "intersection.h"
#include "ray.h"

AppController::AppController(Scene &scene, int width, int height)
    : mScene(scene),
      mWidth(width),
      mHeight(height),
      mSample(),
      mHasUpdate(false),
      mIntegrator(new Depositer())
{
    printf("Beginning pre-process...\n");
    std::clock_t begin = clock();

    mIntegrator->preprocess(mScene, mRandom);

    std::clock_t end = clock();
    double elapsedSeconds = double(end - begin) / CLOCKS_PER_SEC;
    printf("Pre-process complete (%0.1fs elapsed)\n", elapsedSeconds);
}

bool AppController::testAndClearUpdate()
{
    bool ret = mHasUpdate;
    mHasUpdate = false; // race-condition
    return ret;
}

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

    mIntegrator->debug(intersection, mScene);
    printf("Queueing reload...\n");
    mHasUpdate = true;

    // RandomGenerator random;
    // int bounceCount = 10;
    // Sample sample;
    // sample.eyePoints.push_back(ray.origin());

    // Color color = mIntegrator->L(
    //     intersection,
    //     mScene,
    //     random,
    //     bounceCount,
    //     sample
    // );

    // mSample = sample;
}
