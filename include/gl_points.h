#pragma once

#include "gl_scene.h"
#include "gl_types.h"
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

        void updateDebugMode();

    protected:
        std::vector<GLfloat> getPositions();
        std::vector<GLfloat> getColors();

        Shader m_shader;
        EntityIDs m_entityIDs;

        DebugMode m_debugMode;

        int m_pointCount;
        int m_maxPoints;
    };
}
