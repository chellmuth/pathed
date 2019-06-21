#include "gl_points.h"

#include "json.hpp"
using json = nlohmann::json;

#include <assert.h>
#include <fstream>
#include <iostream>

static const int maxPoints = 1000;

gl::Points::Points()
    : mPointCount(0)
{
    mShader = shader::createProgram(
        "shader/point.vs",
        "shader/point.fs"
    );
}

std::vector<GLfloat> gl::Points::getPositions()
{
    std::ifstream jsonScene("live-photons.json");
    json pointsJson = json::parse(jsonScene);

    std::vector<GLfloat> positionsGL;

    positionsGL.push_back(pointsJson["QueryPoint"][0]);
    positionsGL.push_back(pointsJson["QueryPoint"][1]);
    positionsGL.push_back(pointsJson["QueryPoint"][2]);

    mPointCount = 1;

    for (auto &resultJson : pointsJson["Results"]) {
        auto &pointJson = resultJson["point"];
        positionsGL.push_back(pointJson[0]);
        positionsGL.push_back(pointJson[1]);
        positionsGL.push_back(pointJson[2]);

        mPointCount += 1;
    }

    assert(mPointCount <= maxPoints);

    return positionsGL;
}

void gl::Points::init()
{
    std::vector<GLfloat> positionsGL = getPositions();

    {
        glGenVertexArrays(1, &mEntityIDs.vertexArrayID);
        glBindVertexArray(mEntityIDs.vertexArrayID);

        glGenBuffers(1, &mEntityIDs.vertexBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, mEntityIDs.vertexBufferID);
        glBufferData(
            GL_ARRAY_BUFFER,
            sizeof(GLfloat) * maxPoints,
            (GLvoid *)&positionsGL[0],
            GL_DYNAMIC_DRAW
        );
    }
}

void gl::Points::reload()
{
    std::vector<GLfloat> positionsGL = getPositions();

    glBindBuffer(GL_ARRAY_BUFFER, mEntityIDs.vertexBufferID);
    glBufferSubData(
        GL_ARRAY_BUFFER,
        0,
        sizeof(GLfloat) * positionsGL.size(),
        (GLvoid *)&positionsGL[0]
    );
}

void gl::Points::draw(
    GLfloat (&model)[4][4],
    GLfloat (&view)[4][4],
    GLfloat (&projection)[4][4]
) {
    glEnable(GL_PROGRAM_POINT_SIZE);
    glDisable(GL_DEPTH_TEST);

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

    glDrawArrays(GL_POINTS, 0, mPointCount);

    glDisableVertexAttribArray(0);

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_PROGRAM_POINT_SIZE);
}
