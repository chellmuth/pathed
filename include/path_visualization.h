#pragma once

#include "gl_visualization.h"
#include "sample.h"

namespace gl {
    class PathVisualization : public gl::Visualization {
    public:
        void init(const Sample& sample);

        void draw(
            GLfloat (&model)[4][4],
            GLfloat (&view)[4][4],
            GLfloat (&projection)[4][4]
        ) override;
    };
};
