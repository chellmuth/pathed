#pragma once

#include "gl_scene.h"
#include "integrator.h"
#include "point.h"
#include "shader.h"
#include "vector.h"

#include <nanogui/opengl.h>
#include <vector>


namespace gl {
    class Lines {
    public:
        Lines();

        void init();
        void draw(
            GLfloat (&model)[4][4],
            GLfloat (&view)[4][4],
            GLfloat (&projection)[4][4]
        );
        void update(const Sample &sample);

    private:
        Shader m_shader;
        EntityIDs m_entityIDs;
        int m_lineCount;
    };
}
