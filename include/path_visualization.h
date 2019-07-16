#pragma once

#include "gl_visualization.h"

namespace gl {
    class PathVisualization : public gl::Visualization {
    public:
        void draw(
            GLfloat (&model)[4][4],
            GLfloat (&view)[4][4],
            GLfloat (&projection)[4][4]
        ) override;
    };
};
