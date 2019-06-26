#pragma once

#include "gl_scene.h"
#include "shader.h"

#include <nanogui/opengl.h>

namespace gl {
    class Points {
    public:
        Points();

        void init();
        void draw(
            GLfloat (&model)[4][4],
            GLfloat (&view)[4][4],
            GLfloat (&projection)[4][4]
        );

        void reload();

    protected:
        std::vector<GLfloat> getPositions();
        std::vector<GLfloat> getColors();

        Shader m_shader;
        EntityIDs m_entityIDs;

        int m_pointCount;
    };
}
