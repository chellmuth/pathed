#include "gl_points.h"

#include "json.hpp"
using json = nlohmann::json;

#include <fstream>
#include <iostream>

gl::Points::Points() {
    mShader = shader::createProgram(
        "shader/point.vs",
        "shader/point.fs"
    );
}

void gl::Points::init()
{
    std::ifstream jsonScene("live-photons.json");
    json pointsJson = json::parse(jsonScene);

    std::vector<GLfloat> positionsGL;
    std::vector<GLuint> indicesGL;

    positionsGL.push_back(pointsJson["QueryPoint"][0]);
    positionsGL.push_back(pointsJson["QueryPoint"][1]);
    positionsGL.push_back(pointsJson["QueryPoint"][2]);

    std::cout << positionsGL[0] << " " << positionsGL[1] << " " << positionsGL[2] << " " << std::endl;

    indicesGL.push_back(0);
    indicesGL.push_back(1);
    indicesGL.push_back(2);

    {
        glGenVertexArrays(1, &mEntityIDs.vertexArrayID);
        glBindVertexArray(mEntityIDs.vertexArrayID);

        glGenBuffers(1, &mEntityIDs.vertexIndexBufferID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEntityIDs.vertexIndexBufferID);
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            sizeof(GLuint) * indicesGL.size(),
            (GLvoid *)&indicesGL[0],
            GL_STATIC_DRAW
        );

        glGenBuffers(1, &mEntityIDs.vertexBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, mEntityIDs.vertexBufferID);
        glBufferData(
            GL_ARRAY_BUFFER,
            sizeof(GLfloat) * positionsGL.size(),
            (GLvoid *)&positionsGL[0],
            GL_STATIC_DRAW
        );
    }
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

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEntityIDs.vertexIndexBufferID);
    glDrawElements(GL_POINTS, 1, GL_UNSIGNED_INT, 0);

    glDisableVertexAttribArray(0);

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_PROGRAM_POINT_SIZE);
}
