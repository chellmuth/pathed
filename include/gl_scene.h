#pragma once

#include <nanogui/opengl.h>

#include "obj_parser.h"
#include "shader.h"

typedef struct {
    GLuint vertexArrayID;
    GLuint vertexIndexBufferID;
    GLuint vertexBufferID;
    GLuint normalBufferID;
} EntityIDs;

namespace gl {
    class Scene {
    public:
        Scene();

        void init(::Scene &scene);
        void draw();

    protected:
        EntityIDs mEntityIDs;
        int mTriangleCount;
    };
}
