#pragma once

#include "image.h"
#include "scene.h"

#include <nanogui/widget.h>

#include <memory>

class PDFWidget : public nanogui::Widget {
};

std::shared_ptr<Image> renderPDF(
    int phiSteps, int thetaSteps,
    int row, int col,
    const Scene &scene
);
