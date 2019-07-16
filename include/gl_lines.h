#pragma once

#include "gl_scene.h"
#include "integrator.h"
#include "point.h"
#include "shader.h"
#include "vector.h"

#include <nanogui/opengl.h>
#include <vector>


namespace gl {
    struct Line {
        Point3 p0;
        Point3 p1;
    };

    class Lines {
    public:
        Lines();
        ~Lines();

        void init(const std::vector<gl::Line> &lines);
        void draw(
            GLfloat (&model)[4][4],
            GLfloat (&view)[4][4],
            GLfloat (&projection)[4][4]
        );

    private:
        Shader m_shader;
        EntityIDs m_entityIDs;
        int m_lineCount;
    };
}
