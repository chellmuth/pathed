#include "photon_renderer.h"

#include "globals.h"
#include "job.h"
#include "point.h"
#include "vector.h"

#include <assert.h>
#include <fstream>
#include <iostream>
#include <stdlib.h>

using json = nlohmann::json;

gl::PhotonRenderer::PhotonRenderer()
    : m_pointCount(0),
      m_debugMode(DebugMode::Local)
{
    m_shader = shader::createProgram(
        "shader/point.vs",
        "shader/point.fs"
    );
}

void gl::PhotonRenderer::init(const std::string &jsonFile)
{
    std::ifstream jsonScene(jsonFile);
    if (!jsonScene.is_open()) {
        std::cout << "Could not open: " << jsonFile << std::endl;
        exit(1);
    }
    m_pointsJson = json::parse(jsonScene);

    m_pointCount = m_pointsJson["Results"].size() + 1;
    std::cout << "Loading " << m_pointCount << " points" << std::endl;

    {
        glGenVertexArrays(1, &m_entityIDs.vertexArrayID);
        glBindVertexArray(m_entityIDs.vertexArrayID);

        glGenBuffers(1, &m_entityIDs.vertexBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, m_entityIDs.vertexBufferID);
        glBufferData(
            GL_ARRAY_BUFFER,
            sizeof(GLfloat) * 3 * m_pointCount,
            NULL,
            GL_DYNAMIC_DRAW
        );

        glGenBuffers(1, &m_entityIDs.colorBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, m_entityIDs.colorBufferID);
        glBufferData(
            GL_ARRAY_BUFFER,
            sizeof(GLfloat) * 3 * m_pointCount,
            NULL,
            GL_DYNAMIC_DRAW
        );
    }

    updateBuffers();
}

void gl::PhotonRenderer::updateBuffers()
{
    std::vector<GLfloat> positionsGL = getPositions();
    std::vector<GLfloat> colorsGL = getColors();

    glBindBuffer(GL_ARRAY_BUFFER, m_entityIDs.vertexBufferID);
    glBufferSubData(
        GL_ARRAY_BUFFER,
        0,
        sizeof(GLfloat) * positionsGL.size(),
        (GLvoid *)&positionsGL[0]
    );

    glBindBuffer(GL_ARRAY_BUFFER, m_entityIDs.colorBufferID);
    glBufferSubData(
        GL_ARRAY_BUFFER,
        0,
        sizeof(GLfloat) * colorsGL.size(),
        (GLvoid *)&colorsGL[0]
    );
}

std::vector<GLfloat> gl::PhotonRenderer::getPositions()
{
    std::vector<GLfloat> positionsGL;

    Point3 queryPoint(
        m_pointsJson["QueryPoint"][0],
        m_pointsJson["QueryPoint"][1],
        m_pointsJson["QueryPoint"][2]
    );

    for (auto &resultJson : m_pointsJson["Results"]) {
        auto &pointJson = resultJson["point"];

        auto &sourceJson = resultJson["source"];
        Point3 sourcePoint(sourceJson[0], sourceJson[1], sourceJson[2]);

        Vector3 wi = (sourcePoint - queryPoint).toVector().normalized() * 0.2f;
        Point3 hemispherePoint = queryPoint + wi;

        switch(m_debugMode) {
        case DebugMode::Hemisphere: {
            positionsGL.push_back(hemispherePoint.x());
            positionsGL.push_back(hemispherePoint.y());
            positionsGL.push_back(hemispherePoint.z());
            break;
        }
        case DebugMode::Source: {
            positionsGL.push_back(sourcePoint.x());
            positionsGL.push_back(sourcePoint.y());
            positionsGL.push_back(sourcePoint.z());
            break;
        }
        case DebugMode::Local: {
            positionsGL.push_back(pointJson[0]);
            positionsGL.push_back(pointJson[1]);
            positionsGL.push_back(pointJson[2]);
            break;
        }
        }
    }

    positionsGL.push_back(m_pointsJson["QueryPoint"][0]);
    positionsGL.push_back(m_pointsJson["QueryPoint"][1]);
    positionsGL.push_back(m_pointsJson["QueryPoint"][2]);

    return positionsGL;
}

std::vector<GLfloat> gl::PhotonRenderer::getColors()
{
    std::vector<GLfloat> colorsGL;

    for (auto &resultJson : m_pointsJson["Results"]) {
        colorsGL.push_back(0.7f);
        colorsGL.push_back(0.7f);
        colorsGL.push_back(0.7f);
    }

    colorsGL.push_back(1.f);
    colorsGL.push_back(0.f);
    colorsGL.push_back(0.f);

    return colorsGL;
}

void gl::PhotonRenderer::draw(
    GLfloat (&model)[4][4],
    GLfloat (&view)[4][4],
    GLfloat (&projection)[4][4]
) {
    glEnable(GL_PROGRAM_POINT_SIZE);
    glDisable(GL_DEPTH_TEST);

    glUseProgram(m_shader.programID);

    GLuint modelID = glGetUniformLocation(m_shader.programID, "model");
    GLuint viewID = glGetUniformLocation(m_shader.programID, "view");
    GLuint projectionID = glGetUniformLocation(m_shader.programID, "projection");

    glUniformMatrix4fv(modelID, 1, GL_TRUE, &model[0][0]);
    glUniformMatrix4fv(viewID, 1, GL_TRUE, &view[0][0]);
    glUniformMatrix4fv(projectionID, 1, GL_TRUE, &projection[0][0]);

    glBindVertexArray(m_entityIDs.vertexArrayID);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, m_entityIDs.vertexBufferID);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, m_entityIDs.colorBufferID);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    glDrawArrays(GL_POINTS, 0, m_pointCount);

    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_PROGRAM_POINT_SIZE);
}

void gl::PhotonRenderer::updateDebugMode()
{
    switch(m_debugMode) {
    case DebugMode::Hemisphere: {
        m_debugMode = DebugMode::Source;
        break;
    }
    case DebugMode::Source: {
        m_debugMode = DebugMode::Local;
        break;
    }
    case DebugMode::Local: {
        m_debugMode = DebugMode::Hemisphere;
        break;
    }
    }

    updateBuffers();
}
