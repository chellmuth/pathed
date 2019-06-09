#include "gl_lines.h"

gl::Lines::Lines()
{
    mShader = shader::createProgram(
        "shader/geometry.vs",
        "shader/uniform_color.fs"
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
    }

    mLineCount = 0;
}

void gl::Lines::update(const Sample &sample)
{
    if (sample.bounceRays.size() == 0) { return; }

    Point3 source = sample.bounceRays[0];
    Point3 target = sample.shadowRays[0];

    std::vector<GLfloat> positionsGL = {
        // source.x(), source.y(), source.z(),
        // target.x(), target.y(), target.z()
    };

    for (int i = 1; i < sample.bounceRays.size(); i++) {
        Point3 bounceSource = sample.bounceRays[i - 1];
        Point3 bounceTarget = sample.bounceRays[i];
        std::vector<GLfloat> firstBounce = {
            bounceSource.x(), bounceSource.y(), bounceSource.z(),
            bounceTarget.x(), bounceTarget.y(), bounceTarget.z()
        };

        positionsGL.insert(
            positionsGL.end(),
            firstBounce.begin(),
            firstBounce.end()
        );

    }

    glBindBuffer(GL_ARRAY_BUFFER, mEntityIDs.vertexBufferID);
    glBufferSubData(
        GL_ARRAY_BUFFER,
        0,
        sizeof(GLfloat) * positionsGL.size(),
        (GLvoid *)&positionsGL[0]
    );

    mLineCount = sample.bounceRays.size() - 1;
}

void gl::Lines::draw(
    GLfloat (&model)[4][4],
    GLfloat (&view)[4][4],
    GLfloat (&projection)[4][4]
)
{
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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEntityIDs.vertexIndexBufferID);
    glDrawElements(GL_LINES, mLineCount * 2, GL_UNSIGNED_INT, 0);

    glDisableVertexAttribArray(0);
}
