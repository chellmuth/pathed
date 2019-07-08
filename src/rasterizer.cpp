#define  GL_SILENCE_DEPRECATION 1

#include "camera.h"
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
    : mScene(scene), mOrigin(0.f, 0.f, 0.f), mInitialDirection(0.f, 0.f, 0.f),
      nanogui::GLCanvas(parent)
{
    mOrigin = scene.getCamera()->getOrigin();
    mInitialDirection = scene.getCamera()->getTarget() - mOrigin;

    mWidth = width;
    mHeight = height;

    mShader = shader::createProgram(
        "shader/geometry.vs",
        "shader/normal.fs"
    );

    mGLScene.init(scene);
    mGLLines.init();
    mGLPoints.init();

    mArcball.setSize({width, height});
}

void Rasterizer::init()
{
}

void Rasterizer::reload()
{
    mGLPoints.reload();
}

void Rasterizer::setState(const Sample &sample)
{
    mGLLines.update(sample);
}

void Rasterizer::calculateViewMatrix(GLfloat (&view)[4][4])
{
    auto arcballRotation = mArcball.matrix();
    GLfloat arcballRotationLocal[4][4];
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            arcballRotationLocal[row][col] = arcballRotation(row, col);
        }
    }

    GLfloat viewLocal[4][4];
    matrix::makeIdentity(viewLocal);
    Point3 lookAt = mOrigin + mInitialDirection;
    matrix::buildView(
        viewLocal,
        mOrigin.x(), mOrigin.y(), mOrigin.z(),
        lookAt.x(), lookAt.y(), lookAt.z()
    );

    matrix::multiply(view, arcballRotationLocal, viewLocal);
}

void Rasterizer::drawGL()
{
    glEnable(GL_DEPTH_TEST);

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    GLfloat model[4][4];
    matrix::makeIdentity(model);

    GLfloat view[4][4];
    calculateViewMatrix(view);

    float fovRadians = mScene.getCamera()->getVerticalFOV();
    float aspectRatio = 1.f * mWidth / mHeight;

    GLfloat projection[4][4];
    matrix::buildPerspectiveProjection(
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

    mGLScene.draw();

    mGLLines.draw(model, view, projection);
    mGLPoints.draw(model, view, projection);

    glDisable(GL_DEPTH_TEST);
}

void Rasterizer::move(Direction direction)
{
    GLfloat view[4][4];
    calculateViewMatrix(view);

    Eigen::Matrix4f eigenView;
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            eigenView(row, col) = view[row][col];
        }
    }

    switch(direction) {
       case Direction::Forward: {
           eigenView(2, 3) += .1f;
           break;
       }

       case Direction::Backward: {
           eigenView(2, 3) -= .1f;
           break;
       }

       case Direction::Left: {
           eigenView(0, 3) += .1f;
           break;
       }

       case Direction::Right: {
           eigenView(0, 3) -= .1f;
           break;
       }
    }

    auto inverseView = eigenView.inverse();
    mOrigin = Point3(
        inverseView(0, 3),
        inverseView(1, 3),
        inverseView(2, 3)
    );
}

void Rasterizer::updateDebugMode()
{
    mGLPoints.updateDebugMode();
}

bool Rasterizer::mouseButtonEvent(
    const Eigen::Vector2i &p, int button, bool down, int modifiers
) {
    if (button == GLFW_MOUSE_BUTTON_1) {
        mArcball.button(p, down);
        return true;
    }
    return false;
}

bool Rasterizer::mouseMotionEvent(
    const Eigen::Vector2i &p, const Eigen::Vector2i &rel, int button, int modifiers
) {
    mArcball.motion(p);

    return true;
}
