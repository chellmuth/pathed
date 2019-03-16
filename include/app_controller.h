#pragma once

#include "point.h"
#include "scene.h"
#include "vector.h"

#include <vector>

class AppController {
public:
    AppController(Scene &scene, int width, int height);
    void handlePathTraceClick(int x, int y);
    Point3 getSelectedPoint() { return mSelectedPoint; }
    std::vector<Vector3> getIntersections() { return mIntersections; }

private:
    Scene &mScene;
    int mWidth, mHeight;
    Point3 mSelectedPoint;
    std::vector<Vector3> mIntersections;
};
