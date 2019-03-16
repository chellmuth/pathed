#pragma once

#include <nanogui/opengl.h>

#include "gl_scene.h"
#include "point.h"
#include "shader.h"

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
        void updatePoint(Point3 point);

    private:
        Shader mShader;
        EntityIDs mEntityIDs;
        int mLineCount;
    };
}
