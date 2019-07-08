#pragma once

#include <nanogui/opengl.h>

namespace matrix {

    void debugMatrix(GLfloat (&matrix)[4][4]);
    void debugMatrix(GLfloat (&matrix)[4][1]);
    void copyMatrix(GLfloat (&source)[4][4], GLfloat (&target)[4][4]);

    void scale(GLfloat (&result)[4][4], GLfloat x, GLfloat y, GLfloat z);
    void translate(GLfloat (&result)[4][4], GLfloat x, GLfloat y, GLfloat z);
    void rotateX(GLfloat (&result)[4][4], GLfloat theta);
    void rotateY(GLfloat (&result)[4][4], GLfloat theta);
    void rotateZ(GLfloat (&result)[4][4], GLfloat theta);

    void multiply(GLfloat (&result)[4][4], GLfloat (&A)[4][4], GLfloat (&B)[4][4]);
    void multiply(GLfloat (&result)[4][1], GLfloat (&A)[4][4], GLfloat (&x)[4][1]);

    void makeIdentity(GLfloat (&m)[4][4]);

    void buildView(
        GLfloat (&view)[4][4],
        float originX, float originY, float originZ,
        float targetX, float targetY, float targetZ
    );

    void buildOrthographicProjection(
        GLfloat (&projection)[4][4],
        float left, float right, float bottom, float top,
        float zFar, float zNear
    );

    void buildPerspectiveProjection(
        GLfloat (&projection)[4][4],
        float fovY, float aspectRatio, float zFar, float zNear
    );

}
