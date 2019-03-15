#define  GL_SILENCE_DEPRECATION 1

#include "matrix.h"
#include "rasterizer.h"

static void checkError(const char *identifier)
{
    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR)
    {
        printf("THERES AN ERROR: %s, %d\n", identifier, err);

    }
}

Rasterizer::Rasterizer(Widget *parent, Scene &scene, int width, int height)
    : mScene(scene), nanogui::GLCanvas(parent)
{
    mWidth = width;
    mHeight = height;

    mShader = shader::createProgram(
        "shader/geometry.vs",
        "shader/uniform_color.fs"
    );

    mEntity.init(scene);
}

void Rasterizer::init()
{
}

void Rasterizer::drawGL()
{
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    GLfloat model[4][4];
    makeIdentity(model);

    GLfloat view[4][4];
    makeIdentity(view);
    buildView(
        view,
        0.f, 2.f, 15.f,
        0.f, -2.f, 2.5f
    );

    float fovDegrees = 28.f;
    float fovRadians = fovDegrees / 180.f * M_PI;
    float aspectRatio = 1.f * mWidth / mHeight;

    GLfloat projection[4][4];
    buildPerspectiveProjection(
        projection,
        fovRadians, aspectRatio,
        100.f,
        0.1f
    );

    glUseProgram(mShader.programID);

    GLuint modelID = glGetUniformLocation(mShader.programID, "model");
    GLuint viewID = glGetUniformLocation(mShader.programID, "view");
    GLuint projectionID = glGetUniformLocation(mShader.programID, "projection");

    glUniformMatrix4fv(modelID, 1, GL_TRUE, &model[0][0]);
    glUniformMatrix4fv(viewID, 1, GL_TRUE, &view[0][0]);
    glUniformMatrix4fv(projectionID, 1, GL_TRUE, &projection[0][0]);

    mEntity.draw();
}
