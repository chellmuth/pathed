#pragma once

#include <nanogui/opengl.h>

namespace gl {
    class Visualization {
    public:
        virtual ~Visualization() {}

        virtual void draw(
            GLfloat (&model)[4][4],
            GLfloat (&view)[4][4],
            GLfloat (&projection)[4][4]
        ) = 0;
    };
};
