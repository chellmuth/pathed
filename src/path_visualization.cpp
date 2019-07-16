#include "path_visualization.h"

#include <iostream>

void gl::PathVisualization::init(const Sample &sample)
{
    gl::Line line = {
        sample.eyePoints[0],
        sample.eyePoints[1]
    };

    std::vector<gl::Line> lines = { line };

    m_glLines.init(lines);
}

void gl::PathVisualization::draw(
    GLfloat (&model)[4][4],
    GLfloat (&view)[4][4],
    GLfloat (&projection)[4][4]
) {
    m_glLines.draw(model, view, projection);
}
