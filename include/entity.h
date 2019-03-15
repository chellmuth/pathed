#pragma once

#include <nanogui/opengl.h>

#include "obj_parser.h"
#include "shader.h"

typedef struct {
    GLuint vertexArrayID;
    GLuint vertexIndexBufferID;
    GLuint vertexBufferID;
} EntityIDs;

class Entity {
public:
    Entity();

    void init(Scene &scene);
    void draw();

protected:
    EntityIDs mEntityIDs;
    int mTriangleCount;
};
