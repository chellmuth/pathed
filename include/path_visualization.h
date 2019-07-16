#pragma once

#include "gl_lines.h"
#include "gl_visualization.h"
#include "sample.h"

namespace gl {
    class PathVisualization : public gl::Visualization {
    public:
        ~PathVisualization() {};
        void init(const Sample& sample);

        void draw(
            GLfloat (&model)[4][4],
            GLfloat (&view)[4][4],
            GLfloat (&projection)[4][4]
        ) override;

    private:
        gl::Lines m_glLines;
    };
};
