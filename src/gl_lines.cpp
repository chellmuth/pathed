#include "gl_lines.h"

gl::Lines::Lines()
{
    mShader = shader::createProgram(
        "shader/line.vs",
        "shader/line.fs"
    );
}

void gl::Lines::init()
{
    static const int MaxLines = 100;

    GLuint indicesGL[2 * MaxLines];
    for (int i = 0; i < 2 * MaxLines; i++) {
        indicesGL[i] = i;
    }

    {
        glGenVertexArrays(1, &mEntityIDs.vertexArrayID);
        glBindVertexArray(mEntityIDs.vertexArrayID);

        glGenBuffers(1, &mEntityIDs.vertexIndexBufferID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEntityIDs.vertexIndexBufferID);
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            sizeof(GLuint) * 2 * MaxLines,
            (GLvoid *)&indicesGL[0],
            GL_STATIC_DRAW
        );

        glGenBuffers(1, &mEntityIDs.vertexBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, mEntityIDs.vertexBufferID);
        glBufferData(
            GL_ARRAY_BUFFER,
            sizeof(GLfloat) * 3 * 2 * MaxLines,
            NULL,
            GL_DYNAMIC_DRAW
        );

        glGenBuffers(1, &mEntityIDs.colorBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, mEntityIDs.colorBufferID);
        glBufferData(
            GL_ARRAY_BUFFER,
            sizeof(GLfloat) * 3 * MaxLines,
            NULL,
            GL_DYNAMIC_DRAW
        );
    }

    mLineCount = 0;
}

void gl::Lines::update(const Sample &sample)
{
    mLineCount = 0;

    if (sample.lightPoints.size() < 2 && sample.eyePoints.size() < 2) { return; }

    std::vector<GLfloat> positionsGL = {};
    std::vector<GLfloat> colorsGL = {};

    for (int i = 1; i < sample.eyePoints.size(); i++) {
        Point3 bounceSource = sample.eyePoints[i - 1];
        Point3 bounceTarget = sample.eyePoints[i];

        std::vector<GLfloat> bounce = {
            bounceSource.x(), bounceSource.y(), bounceSource.z(),
            bounceTarget.x(), bounceTarget.y(), bounceTarget.z()
        };

        positionsGL.insert(
            positionsGL.end(),
            bounce.begin(),
            bounce.end()
        );

        for (int i = 0; i < 2; i++) {
            colorsGL.push_back(1.f);
            colorsGL.push_back(1.f);
            colorsGL.push_back(1.f);
        }

        mLineCount += 1;
    }

    for (int i = 1; i < sample.lightPoints.size(); i++) {
        Point3 bounceSource = sample.lightPoints[i - 1];
        Point3 bounceTarget = sample.lightPoints[i];

        std::vector<GLfloat> bounce = {
            bounceSource.x(), bounceSource.y(), bounceSource.z(),
            bounceTarget.x(), bounceTarget.y(), bounceTarget.z()
        };

        positionsGL.insert(
            positionsGL.end(),
            bounce.begin(),
            bounce.end()
        );

        for (int i = 0; i < 2; i++) {
            colorsGL.push_back(1.f);
            colorsGL.push_back(1.f);
            colorsGL.push_back(0.f);
        }

        mLineCount += 1;
    }

    if (sample.connected) {
        Point3 bounceSource = sample.eyePoints[sample.eyePoints.size() - 1];
        Point3 bounceTarget = sample.lightPoints[sample.lightPoints.size() - 1];

        std::vector<GLfloat> bounce = {
            bounceSource.x(), bounceSource.y(), bounceSource.z(),
            bounceTarget.x(), bounceTarget.y(), bounceTarget.z()
        };

        positionsGL.insert(
            positionsGL.end(),
            bounce.begin(),
            bounce.end()
        );

        for (int i = 0; i < 2; i++) {
            colorsGL.push_back(0.f);
            colorsGL.push_back(1.f);
            colorsGL.push_back(1.f);
        }

        mLineCount += 1;
    }

    glBindBuffer(GL_ARRAY_BUFFER, mEntityIDs.vertexBufferID);
    glBufferSubData(
        GL_ARRAY_BUFFER,
        0,
        sizeof(GLfloat) * positionsGL.size(),
        (GLvoid *)&positionsGL[0]
    );

    glBindBuffer(GL_ARRAY_BUFFER, mEntityIDs.colorBufferID);
    glBufferSubData(
        GL_ARRAY_BUFFER,
        0,
        sizeof(GLfloat) * colorsGL.size(),
        (GLvoid *)&colorsGL[0]
    );
}

void gl::Lines::draw(
    GLfloat (&model)[4][4],
    GLfloat (&view)[4][4],
    GLfloat (&projection)[4][4]
) {
    glUseProgram(mShader.programID);

    GLuint modelID = glGetUniformLocation(mShader.programID, "model");
    GLuint viewID = glGetUniformLocation(mShader.programID, "view");
    GLuint projectionID = glGetUniformLocation(mShader.programID, "projection");

    glUniformMatrix4fv(modelID, 1, GL_TRUE, &model[0][0]);
    glUniformMatrix4fv(viewID, 1, GL_TRUE, &view[0][0]);
    glUniformMatrix4fv(projectionID, 1, GL_TRUE, &projection[0][0]);

    glBindVertexArray(mEntityIDs.vertexArrayID);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, mEntityIDs.vertexBufferID);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, mEntityIDs.colorBufferID);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEntityIDs.vertexIndexBufferID);
    glDrawElements(GL_LINES, mLineCount * 2, GL_UNSIGNED_INT, 0);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}
