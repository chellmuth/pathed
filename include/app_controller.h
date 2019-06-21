#pragma once

#include "integrator.h"
#include "point.h"
#include "random_generator.h"
#include "scene.h"
#include "vector.h"

#include <memory>
#include <vector>

class AppController {
public:
    AppController(Scene &scene, int width, int height);
    bool testAndClearUpdate();
    void handlePathTraceClick(int x, int y);
    Sample getSample() { return mSample; }

private:
    std::unique_ptr<Integrator> mIntegrator;
    RandomGenerator mRandom;
    Scene &mScene;
    int mWidth, mHeight;
    Sample mSample;

    bool mHasUpdate;
};
