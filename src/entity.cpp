#include "entity.h"

Entity::Entity() {}

void Entity::init(Scene &scene)
{
    auto surfaces = scene.getSurfaces();
    std::vector<GLfloat> positionsGL;
    std::vector<GLuint> indicesGL;

    for (int i = 0; i < surfaces.size(); i++) {
        int offset = positionsGL.size() / 3;

        auto shape = surfaces[i]->getShape();
        shape->pushVertices(positionsGL);
        shape->pushIndices(indicesGL, offset);
    }

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

    mTriangleCount = indicesGL.size() / 3; 
}

void Entity::draw()
{
    glBindVertexArray(mEntityIDs.vertexArrayID);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, mEntityIDs.vertexBufferID);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEntityIDs.vertexIndexBufferID);
    glDrawElements(GL_TRIANGLES, mTriangleCount * 3, GL_UNSIGNED_INT, 0);

    glDisableVertexAttribArray(0);
}
