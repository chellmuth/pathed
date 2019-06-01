#pragma once

#include "integrator.h"
#include "point.h"
#include "scene.h"
#include "vector.h"

#include <vector>

class AppController {
public:
    AppController(Scene &scene, int width, int height);
    void handlePathTraceClick(int x, int y);
    Sample getSample() { return mSample; }

private:
    Scene &mScene;
    int mWidth, mHeight;
    Sample mSample;
};
