#include "gl_points.h"

#include "point.h"
#include "vector.h"

#include "json.hpp"
using json = nlohmann::json;

#include <assert.h>
#include <fstream>
#include <iostream>

static const int maxPoints = 10000;

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
    mPointCount = 0;

    std::ifstream jsonScene("live-photons.json");
    json pointsJson = json::parse(jsonScene);

    std::vector<GLfloat> positionsGL;

    Point3 queryPoint(
        pointsJson["QueryPoint"][0],
        pointsJson["QueryPoint"][1],
        pointsJson["QueryPoint"][2]
    );

    for (auto &resultJson : pointsJson["Results"]) {
        auto &pointJson = resultJson["point"];
        // positionsGL.push_back(pointJson[0]);
        // positionsGL.push_back(pointJson[1]);
        // positionsGL.push_back(pointJson[2]);

        auto &sourceJson = resultJson["source"];
        Point3 sourcePoint(sourceJson[0], sourceJson[1], sourceJson[2]);

        // positionsGL.push_back(sourcePoint.x());
        // positionsGL.push_back(sourcePoint.y());
        // positionsGL.push_back(sourcePoint.z());

        Vector3 wi = (sourcePoint - queryPoint).toVector().normalized() * 0.2f;
        Point3 hemispherePoint = queryPoint + wi;

        positionsGL.push_back(hemispherePoint.x());
        positionsGL.push_back(hemispherePoint.y());
        positionsGL.push_back(hemispherePoint.z());

        mPointCount += 1;
    }

    positionsGL.push_back(pointsJson["QueryPoint"][0]);
    positionsGL.push_back(pointsJson["QueryPoint"][1]);
    positionsGL.push_back(pointsJson["QueryPoint"][2]);

    mPointCount += 1;

    assert(mPointCount <= maxPoints);

    return positionsGL;
}

std::vector<GLfloat> gl::Points::getColors()
{
    std::ifstream jsonScene("live-photons.json");
    json pointsJson = json::parse(jsonScene);

    std::vector<GLfloat> colorsGL;

    for (auto &resultJson : pointsJson["Results"]) {
        colorsGL.push_back(0.7f);
        colorsGL.push_back(0.7f);
        colorsGL.push_back(0.7f);
    }

    colorsGL.push_back(1.f);
    colorsGL.push_back(0.f);
    colorsGL.push_back(0.f);

    return colorsGL;
}

void gl::Points::init()
{
    {
        glGenVertexArrays(1, &mEntityIDs.vertexArrayID);
        glBindVertexArray(mEntityIDs.vertexArrayID);

        glGenBuffers(1, &mEntityIDs.vertexBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, mEntityIDs.vertexBufferID);
        glBufferData(
            GL_ARRAY_BUFFER,
            sizeof(GLfloat) * maxPoints,
            NULL,
            GL_DYNAMIC_DRAW
        );

        glGenBuffers(1, &mEntityIDs.colorBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, mEntityIDs.colorBufferID);
        glBufferData(
            GL_ARRAY_BUFFER,
            sizeof(GLfloat) * maxPoints,
            NULL,
            GL_DYNAMIC_DRAW
        );
    }

    reload();
}

void gl::Points::reload()
{
    std::vector<GLfloat> positionsGL = getPositions();
    std::vector<GLfloat> colorsGL = getColors();

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

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, mEntityIDs.colorBufferID);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    glDrawArrays(GL_POINTS, 0, mPointCount);

    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_PROGRAM_POINT_SIZE);
}
