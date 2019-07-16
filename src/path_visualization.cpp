#include "path_visualization.h"

#include <iostream>

void gl::PathVisualization::init(const Sample &sample)
{
    std::vector<gl::Line> lines;

    for (int i = 0; i < sample.eyePoints.size() - 1; i++) {
        gl::Line line = {
            sample.eyePoints[i],
            sample.eyePoints[i + 1]
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
