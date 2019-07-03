#pragma once

#include "obj_parser.h"
#include "scene.h"
#include "shader.h"

#include <nanogui/opengl.h>

typedef struct {
    GLuint vertexArrayID;
    GLuint vertexIndexBufferID;
    GLuint vertexBufferID;
    GLuint colorBufferID;
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
