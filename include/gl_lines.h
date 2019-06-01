#pragma once

#include <nanogui/opengl.h>
#include <vector>

#include "integrator.h"
#include "gl_scene.h"
#include "point.h"
#include "shader.h"
#include "vector.h"

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
        Shader mShader;
        EntityIDs mEntityIDs;
        int mLineCount;
    };
}
