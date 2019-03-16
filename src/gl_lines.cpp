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
    std::vector<GLfloat> positionsGL = {
        1.25f, 0.f, 0.f,
        -3.75f, 0.f, 0.f,
    };
    std::vector<GLuint> indicesGL = { 0, 1 };

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

    mLineCount = 1;
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
