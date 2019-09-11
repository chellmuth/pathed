#pragma once

#include "image.h"
#include "integrator.h"
#include "intersection.h"
#include "scene.h"
#include "render_status.h"

#include <vector>

class PDFIntegrator : public Integrator {
public:
    void run(
        Image &image,
        Scene &scene,
        std::function<void(RenderStatus)> callback,
        bool *quit
    ) override;

protected:
    void renderPDF(
        std::vector<float> &radianceLookup,
        const Scene &scene,
        const Intersection &intersection
    );

};
