#include "gl_lines.h"

gl::Lines::Lines()
{
    m_shader = shader::createProgram(
        "shader/line.vs",
        "shader/line.fs"
    );
}

gl::Lines::~Lines()
{
    glDeleteVertexArrays(1, &m_entityIDs.vertexArrayID);
    glDeleteBuffers(1, &m_entityIDs.vertexIndexBufferID);
    glDeleteBuffers(1, &m_entityIDs.vertexBufferID);
    glDeleteBuffers(1, &m_entityIDs.colorBufferID);
}

void gl::Lines::init(const std::vector<gl::Line> &lines)
{
    m_lineCount = lines.size();
    const int pointCount = 2 * m_lineCount;

    std::vector<GLuint> indicesGL(pointCount);
    for (int i = 0; i < pointCount; i++) {
        indicesGL[i] = i;
    }

    std::vector<GLfloat> positionsGL(pointCount * 3);
    std::vector<GLfloat> colorsGL(pointCount * 3);
    for (int i = 0; i < m_lineCount; i++) {
        positionsGL[6 * i + 0] = lines[i].p0.x();
        positionsGL[6 * i + 1] = lines[i].p0.y();
        positionsGL[6 * i + 2] = lines[i].p0.z();

        positionsGL[6 * i + 3] = lines[i].p1.x();
        positionsGL[6 * i + 4] = lines[i].p1.y();
        positionsGL[6 * i + 5] = lines[i].p1.z();

        colorsGL[6 * i + 0] = 1.f;
        colorsGL[6 * i + 1] = 1.f;
        colorsGL[6 * i + 2] = 0.f;

        colorsGL[6 * i + 3] = 1.f;
        colorsGL[6 * i + 4] = 1.f;
        colorsGL[6 * i + 5] = 0.f;
    }

    {
        glGenVertexArrays(1, &m_entityIDs.vertexArrayID);
        glBindVertexArray(m_entityIDs.vertexArrayID);

        glGenBuffers(1, &m_entityIDs.vertexIndexBufferID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_entityIDs.vertexIndexBufferID);
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            sizeof(GLuint) * pointCount,
            (GLvoid *)&indicesGL[0],
            GL_STATIC_DRAW
        );

        glGenBuffers(1, &m_entityIDs.vertexBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, m_entityIDs.vertexBufferID);
        glBufferData(
            GL_ARRAY_BUFFER,
            sizeof(GLfloat) * 3 * pointCount,
            (GLvoid *)&positionsGL[0],
            GL_STATIC_DRAW
        );

        glGenBuffers(1, &m_entityIDs.colorBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, m_entityIDs.colorBufferID);
        glBufferData(
            GL_ARRAY_BUFFER,
            sizeof(GLfloat) * 3 * pointCount,
            (GLvoid *)&colorsGL[0],
            GL_STATIC_DRAW
        );
    }
}

// void gl::Lines::update(const Sample &sample)
// {
//     m_lineCount = 0;

//     if (sample.lightPoints.size() < 2 && sample.eyePoints.size() < 2) { return; }

//     std::vector<GLfloat> positionsGL = {};
//     std::vector<GLfloat> colorsGL = {};

//     for (int i = 1; i < sample.eyePoints.size(); i++) {
//         Point3 bounceSource = sample.eyePoints[i - 1];
//         Point3 bounceTarget = sample.eyePoints[i];

//         std::vector<GLfloat> bounce = {
//             bounceSource.x(), bounceSource.y(), bounceSource.z(),
//             bounceTarget.x(), bounceTarget.y(), bounceTarget.z()
//         };

//         positionsGL.insert(
//             positionsGL.end(),
//             bounce.begin(),
//             bounce.end()
//         );

//         for (int i = 0; i < 2; i++) {
//             colorsGL.push_back(1.f);
//             colorsGL.push_back(1.f);
//             colorsGL.push_back(1.f);
//         }

//         m_lineCount += 1;
//     }

//     for (int i = 0; i < sample.shadowPoints.size(); i++) {
//         Point3 bounceSource = sample.eyePoints[i + 1];
//         Point3 bounceTarget = sample.shadowPoints[i];

//         std::vector<GLfloat> bounce = {
//             bounceSource.x(), bounceSource.y(), bounceSource.z(),
//             bounceTarget.x(), bounceTarget.y(), bounceTarget.z()
//         };

//         positionsGL.insert(
//             positionsGL.end(),
//             bounce.begin(),
//             bounce.end()
//         );

//         for (int i = 0; i < 2; i++) {
//             colorsGL.push_back(0.f);
//             colorsGL.push_back(0.f);
//             colorsGL.push_back(0.f);
//         }

//         m_lineCount += 1;
//     }

//     for (int i = 1; i < sample.lightPoints.size(); i++) {
//         Point3 bounceSource = sample.lightPoints[i - 1];
//         Point3 bounceTarget = sample.lightPoints[i];

//         std::vector<GLfloat> bounce = {
//             bounceSource.x(), bounceSource.y(), bounceSource.z(),
//             bounceTarget.x(), bounceTarget.y(), bounceTarget.z()
//         };

//         positionsGL.insert(
//             positionsGL.end(),
//             bounce.begin(),
//             bounce.end()
//         );

//         for (int i = 0; i < 2; i++) {
//             colorsGL.push_back(1.f);
//             colorsGL.push_back(1.f);
//             colorsGL.push_back(0.f);
//         }

//         m_lineCount += 1;
//     }

//     if (sample.connected) {
//         Point3 bounceSource = sample.eyePoints[sample.eyePoints.size() - 1];
//         Point3 bounceTarget = sample.lightPoints[sample.lightPoints.size() - 1];

//         std::vector<GLfloat> bounce = {
//             bounceSource.x(), bounceSource.y(), bounceSource.z(),
//             bounceTarget.x(), bounceTarget.y(), bounceTarget.z()
//         };

//         positionsGL.insert(
//             positionsGL.end(),
//             bounce.begin(),
//             bounce.end()
//         );

//         for (int i = 0; i < 2; i++) {
//             colorsGL.push_back(0.f);
//             colorsGL.push_back(1.f);
//             colorsGL.push_back(1.f);
//         }

//         m_lineCount += 1;
//     }

//     glBindBuffer(GL_ARRAY_BUFFER, m_entityIDs.vertexBufferID);
//     glBufferSubData(
//         GL_ARRAY_BUFFER,
//         0,
//         sizeof(GLfloat) * positionsGL.size(),
//         (GLvoid *)&positionsGL[0]
//     );

//     glBindBuffer(GL_ARRAY_BUFFER, m_entityIDs.colorBufferID);
//     glBufferSubData(
//         GL_ARRAY_BUFFER,
//         0,
//         sizeof(GLfloat) * colorsGL.size(),
//         (GLvoid *)&colorsGL[0]
//     );
// }

void gl::Lines::draw(
    GLfloat (&model)[4][4],
    GLfloat (&view)[4][4],
    GLfloat (&projection)[4][4]
) {
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

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_entityIDs.vertexIndexBufferID);
    glDrawElements(GL_LINES, m_lineCount * 2, GL_UNSIGNED_INT, 0);

    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);
}
