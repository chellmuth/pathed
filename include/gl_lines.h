#pragma once

#include <nanogui/opengl.h>
#include <vector>

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
        void update(Point3 point, std::vector<Vector3> intersections);

    private:
        Shader mShader;
        EntityIDs mEntityIDs;
        int mLineCount;
    };
}
