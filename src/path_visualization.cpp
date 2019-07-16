#include "path_visualization.h"

void gl::PathVisualization::init(const Sample &sample)
{
    std::vector<gl::Line> lines;

    for (int i = 0; i < sample.eyePoints.size() - 1; i++) {
        gl::Line line = {
            sample.eyePoints[i],
            sample.eyePoints[i + 1],
            Color(1.f, 0.f, 0.f)
        };

        lines.push_back(line);
    }

    for (auto &shadowTest : sample.shadowTests) {
        if (shadowTest.occluded) { continue; }

        gl::Line line = {
            shadowTest.shadingPoint,
            shadowTest.lightPoint,
            Color(1.f, 1.f, 0.f)
        };

        lines.push_back(line);
    }

    m_glLines.init(lines);
}

void gl::PathVisualization::draw(
    GLfloat (&model)[4][4],
    GLfloat (&view)[4][4],
    GLfloat (&projection)[4][4]
) {
    m_glLines.draw(model, view, projection);
}
