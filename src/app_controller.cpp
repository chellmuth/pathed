#include "app_controller.h"

#include "bdpt.h"
#include "bounce_controller.h"
#include "camera.h"
#include "depositer.h"
#include "globals.h"
#include "intersection.h"
#include "job.h"
#include "nearest_photon.h"
#include "path_tracer.h"
#include "ray.h"
#include "visualization.h"

#include <ctime>
#include <iostream>

AppController::AppController(Scene &scene, int width, int height)
    : m_scene(scene),
      m_width(width),
      m_height(height),
      m_sample(),
      m_hasUpdate(false),
      m_integrator(std::make_unique<NearestPhoton>())
{
    m_visualizationFiles = visualization::files();

    printf("Beginning pre-process...\n");
    std::clock_t begin = clock();

    m_integrator->preprocess(m_scene, m_random);

    std::clock_t end = clock();
    double elapsedSeconds = double(end - begin) / CLOCKS_PER_SEC;
    printf("Pre-process complete (%0.1fs elapsed)\n", elapsedSeconds);
}

bool AppController::testAndClearUpdate()
{
    bool ret = m_hasUpdate;
    m_hasUpdate = false; // race-condition
    return ret;
}

void AppController::handlePathTraceClick(int x, int y)
{
    int row = (m_height - 1) - y;
    int col = x;

    Ray ray = m_scene.getCamera()->generateRay(
        row, col,
        m_width, m_height
    );

    Intersection intersection = m_scene.testIntersect(ray);
    if (!intersection.hit) { return; }

    m_integrator->debug(intersection, m_scene);

    printf("Queueing reload...\n");
    m_visualizationFiles = visualization::files();
    m_hasUpdate = true;

    // RandomGenerator random;
    // int bounceCount = 10;
    // Sample sample;
    // sample.eyePoints.push_back(ray.origin());

    // Color color = m_integrator->L(
    //     intersection,
    //     m_scene,
    //     random,
    //     bounceCount,
    //     sample
    // );

    // m_sample = sample;
}
