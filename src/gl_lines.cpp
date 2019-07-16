#include "gl_lines.h"

gl::Lines::Lines()
{
    m_shader = shader::createProgram(
        "shader/line.vs",
        "shader/line.fs"
    );
}

void gl::Lines::init()
{
    // static const int MaxLines = 100;

    // GLuint indicesGL[2 * MaxLines];
    // for (int i = 0; i < 2 * MaxLines; i++) {
    //     indicesGL[i] = i;
    // }

    // {
    //     glGenVertexArrays(1, &m_entityIDs.vertexArrayID);
    //     glBindVertexArray(m_entityIDs.vertexArrayID);

    //     glGenBuffers(1, &m_entityIDs.vertexIndexBufferID);
    //     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_entityIDs.vertexIndexBufferID);
    //     glBufferData(
    //         GL_ELEMENT_ARRAY_BUFFER,
    //         sizeof(GLuint) * 2 * MaxLines,
    //         (GLvoid *)&indicesGL[0],
    //         GL_STATIC_DRAW
    //     );

    //     glGenBuffers(1, &m_entityIDs.vertexBufferID);
    //     glBindBuffer(GL_ARRAY_BUFFER, m_entityIDs.vertexBufferID);
    //     glBufferData(
    //         GL_ARRAY_BUFFER,
    //         sizeof(GLfloat) * 3 * 2 * MaxLines,
    //         NULL,
    //         GL_DYNAMIC_DRAW
    //     );

    //     glGenBuffers(1, &m_entityIDs.colorBufferID);
    //     glBindBuffer(GL_ARRAY_BUFFER, m_entityIDs.colorBufferID);
    //     glBufferData(
    //         GL_ARRAY_BUFFER,
    //         sizeof(GLfloat) * 3 * MaxLines,
    //         NULL,
    //         GL_DYNAMIC_DRAW
    //     );
    // }

    // m_lineCount = 0;
}

void gl::Lines::update(const Sample &sample)
{
    // m_lineCount = 0;

    // if (sample.lightPoints.size() < 2 && sample.eyePoints.size() < 2) { return; }

    // std::vector<GLfloat> positionsGL = {};
    // std::vector<GLfloat> colorsGL = {};

    // for (int i = 1; i < sample.eyePoints.size(); i++) {
    //     Point3 bounceSource = sample.eyePoints[i - 1];
    //     Point3 bounceTarget = sample.eyePoints[i];

    //     std::vector<GLfloat> bounce = {
    //         bounceSource.x(), bounceSource.y(), bounceSource.z(),
    //         bounceTarget.x(), bounceTarget.y(), bounceTarget.z()
    //     };

    //     positionsGL.insert(
    //         positionsGL.end(),
    //         bounce.begin(),
    //         bounce.end()
    //     );

    //     for (int i = 0; i < 2; i++) {
    //         colorsGL.push_back(1.f);
    //         colorsGL.push_back(1.f);
    //         colorsGL.push_back(1.f);
    //     }

    //     m_lineCount += 1;
    // }

    // for (int i = 0; i < sample.shadowPoints.size(); i++) {
    //     Point3 bounceSource = sample.eyePoints[i + 1];
    //     Point3 bounceTarget = sample.shadowPoints[i];

    //     std::vector<GLfloat> bounce = {
    //         bounceSource.x(), bounceSource.y(), bounceSource.z(),
    //         bounceTarget.x(), bounceTarget.y(), bounceTarget.z()
    //     };

    //     positionsGL.insert(
    //         positionsGL.end(),
    //         bounce.begin(),
    //         bounce.end()
    //     );

    //     for (int i = 0; i < 2; i++) {
    //         colorsGL.push_back(0.f);
    //         colorsGL.push_back(0.f);
    //         colorsGL.push_back(0.f);
    //     }

    //     m_lineCount += 1;
    // }

    // for (int i = 1; i < sample.lightPoints.size(); i++) {
    //     Point3 bounceSource = sample.lightPoints[i - 1];
    //     Point3 bounceTarget = sample.lightPoints[i];

    //     std::vector<GLfloat> bounce = {
    //         bounceSource.x(), bounceSource.y(), bounceSource.z(),
    //         bounceTarget.x(), bounceTarget.y(), bounceTarget.z()
    //     };

    //     positionsGL.insert(
    //         positionsGL.end(),
    //         bounce.begin(),
    //         bounce.end()
    //     );

    //     for (int i = 0; i < 2; i++) {
    //         colorsGL.push_back(1.f);
    //         colorsGL.push_back(1.f);
    //         colorsGL.push_back(0.f);
    //     }

    //     m_lineCount += 1;
    // }

    // if (sample.connected) {
    //     Point3 bounceSource = sample.eyePoints[sample.eyePoints.size() - 1];
    //     Point3 bounceTarget = sample.lightPoints[sample.lightPoints.size() - 1];

    //     std::vector<GLfloat> bounce = {
    //         bounceSource.x(), bounceSource.y(), bounceSource.z(),
    //         bounceTarget.x(), bounceTarget.y(), bounceTarget.z()
    //     };

    //     positionsGL.insert(
    //         positionsGL.end(),
    //         bounce.begin(),
    //         bounce.end()
    //     );

    //     for (int i = 0; i < 2; i++) {
    //         colorsGL.push_back(0.f);
    //         colorsGL.push_back(1.f);
    //         colorsGL.push_back(1.f);
    //     }

    //     m_lineCount += 1;
    // }

    // glBindBuffer(GL_ARRAY_BUFFER, m_entityIDs.vertexBufferID);
    // glBufferSubData(
    //     GL_ARRAY_BUFFER,
    //     0,
    //     sizeof(GLfloat) * positionsGL.size(),
    //     (GLvoid *)&positionsGL[0]
    // );

    // glBindBuffer(GL_ARRAY_BUFFER, m_entityIDs.colorBufferID);
    // glBufferSubData(
    //     GL_ARRAY_BUFFER,
    //     0,
    //     sizeof(GLfloat) * colorsGL.size(),
    //     (GLvoid *)&colorsGL[0]
    // );
}

void gl::Lines::draw(
    GLfloat (&model)[4][4],
    GLfloat (&view)[4][4],
    GLfloat (&projection)[4][4]
) {
    // glUseProgram(m_shader.programID);

    // GLuint modelID = glGetUniformLocation(m_shader.programID, "model");
    // GLuint viewID = glGetUniformLocation(m_shader.programID, "view");
    // GLuint projectionID = glGetUniformLocation(m_shader.programID, "projection");

    // glUniformMatrix4fv(modelID, 1, GL_TRUE, &model[0][0]);
    // glUniformMatrix4fv(viewID, 1, GL_TRUE, &view[0][0]);
    // glUniformMatrix4fv(projectionID, 1, GL_TRUE, &projection[0][0]);

    // glBindVertexArray(m_entityIDs.vertexArrayID);

    // glEnableVertexAttribArray(0);
    // glBindBuffer(GL_ARRAY_BUFFER, m_entityIDs.vertexBufferID);
    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    // glEnableVertexAttribArray(1);
    // glBindBuffer(GL_ARRAY_BUFFER, m_entityIDs.colorBufferID);
    // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    // glEnableVertexAttribArray(1);
    // glBindBuffer(GL_ARRAY_BUFFER, m_entityIDs.colorBufferID);
    // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_entityIDs.vertexIndexBufferID);
    // glDrawElements(GL_LINES, m_lineCount * 2, GL_UNSIGNED_INT, 0);

    // glDisableVertexAttribArray(0);
    // glDisableVertexAttribArray(1);
}
