#include "matrix.h"

#include "vector.h"

#include <math.h>

void matrix::debugMatrix(GLfloat (&matrix)[4][4])
{
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            printf("%f ", matrix[i][j]);
        }
        printf("\n");
    }
}

void matrix::debugMatrix(GLfloat (&matrix)[4][1])
{
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 1; j++) {
            printf("%f ", matrix[i][j]);
        }
        printf("\n");
    }
}

void matrix::copyMatrix(GLfloat (&source)[4][4], GLfloat (&target)[4][4])
{
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            target[i][j] = source[i][j];
        }
    }
}

void matrix::scale(GLfloat (&result)[4][4], GLfloat x, GLfloat y, GLfloat z)
{
    GLfloat scaled[4][4];
    makeIdentity(scaled);

    scaled[0][0] = x;
    scaled[1][1] = y;
    scaled[2][2] = z;

    GLfloat original[4][4];
    copyMatrix(result, original);
    multiply(result, scaled, original);
}

void matrix::translate(GLfloat (&result)[4][4], GLfloat x, GLfloat y, GLfloat z)
{
    GLfloat translation[4][4];
    makeIdentity(translation);

    translation[0][3] = x;
    translation[1][3] = y;
    translation[2][3] = z;

    GLfloat original[4][4];
    copyMatrix(result, original);
    multiply(result, translation, original);
}

void matrix::multiply(GLfloat (&result)[4][4], GLfloat (&A)[4][4], GLfloat (&B)[4][4])
{
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            result[row][col] = 0.f;

            for (int i = 0; i < 4; i++) {
                result[row][col] += A[row][i] * B[i][col];
            }
        }
    }
}

void matrix::multiply(GLfloat (&result)[4][1], GLfloat (&A)[4][4], GLfloat (&x)[4][1])
{
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 1; col++) {
            result[row][col] = 0.f;

            for (int i = 0; i < 4; i++) {
                result[row][col] += A[row][i] * x[i][col];
            }
        }
    }
}

void matrix::rotateX(GLfloat (&result)[4][4], GLfloat theta)
{
    GLfloat rotation[4][4];
    makeIdentity(rotation);

    rotation[1][1] = cosf(theta);
    rotation[1][2] = -sinf(theta);
    rotation[2][1] = sinf(theta);
    rotation[2][2] = cosf(theta);

    GLfloat original[4][4];
    copyMatrix(result, original);
    multiply(result, rotation, original);
}

void matrix::rotateY(GLfloat (&result)[4][4], GLfloat theta)
{
    GLfloat rotation[4][4];
    makeIdentity(rotation);

    rotation[0][0] = cosf(theta);
    rotation[0][2] = sinf(theta);
    rotation[2][0] = -sinf(theta);
    rotation[2][2] = cosf(theta);

    GLfloat original[4][4];
    copyMatrix(result, original);
    multiply(result, rotation, original);
}

void matrix::rotateZ(GLfloat (&result)[4][4], GLfloat theta)
{
    GLfloat rotation[4][4];
    makeIdentity(rotation);

    rotation[0][0] = cosf(theta);
    rotation[0][1] = -sinf(theta);
    rotation[1][0] = sinf(theta);
    rotation[1][1] = cosf(theta);

    GLfloat original[4][4];
    copyMatrix(result, original);
    multiply(result, rotation, original);
}

void matrix::makeIdentity(GLfloat (&m)[4][4])
{
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            m[row][col] = row == col ? 1.f : 0.f;
        }
    }
}

void matrix::buildView(
    GLfloat (&view)[4][4],
    float originX, float originY, float originZ,
    float targetX, float targetY, float targetZ
)
{
    Vector3 lookAt = Vector3(
        targetX - originX,
        targetY - originY,
        targetZ - originZ
    ).normalized();

    Vector3 up(0.f, 1.f, 0.f);
    Vector3 tangent = up.cross(lookAt).normalized();
    Vector3 bitangent = lookAt.cross(tangent).normalized();

    GLfloat basis[4][4] = {
        tangent.x(), tangent.y(), tangent.z(), 0.f,
        bitangent.x(), bitangent.y(), bitangent.z(), 0.f,
        -lookAt.x(), -lookAt.y(), -lookAt.z(), 0.f,
        0.f, 0.f, 0.f, 1.f
    };

    GLfloat translation[4][4];
    makeIdentity(translation);
    translate(translation, -originX, -originY, -originZ);
    multiply(view, basis, translation);
}

void matrix::buildOrthographicProjection(
    GLfloat (&projection)[4][4],
    float left, float right, float bottom, float top,
    float zFar, float zNear
) {
    makeIdentity(projection);

    projection[0][0] = 2.f / (right - left);
    projection[1][1] = 2.f / (top - bottom);
    projection[2][2] = -2.f / (zFar - zNear);
    projection[3][3] = 1.f;

    projection[0][3] = -(right + left) / (right - left);
    projection[1][3] = -(top + bottom) / (top - bottom);
    projection[2][3] = -(zFar + zNear) / (zFar - zNear);
}

void matrix::buildPerspectiveProjection(
    GLfloat (&projection)[4][4],
    float fovY, float aspectRatio, float zFar, float zNear
) {
    makeIdentity(projection);

    float tanHalfFovy = tan(fovY / 2.0);

    projection[0][0] = 1.f / (aspectRatio * tanHalfFovy);
    projection[1][1] = 1.f / (tanHalfFovy);
    projection[2][2] = -(zFar + zNear) / (zFar - zNear);

    projection[3][2] = -1.f;
    projection[2][3] = -(2.f * zFar * zNear) / (zFar - zNear);
}
