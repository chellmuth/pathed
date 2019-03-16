#pragma once

#include "point.h"
#include "scene.h"

class AppController {
public:
    AppController(Scene &scene, int width, int height);
    void handlePathTraceClick(int x, int y);
    Point3 getSelectedPoint() { return mSelectedPoint; }

private:
    Scene &mScene;
    int mWidth, mHeight;
    Point3 mSelectedPoint;
};
