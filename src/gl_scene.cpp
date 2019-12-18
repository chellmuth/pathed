#include "gl_scene.h"

gl::Scene::Scene() {}

void gl::Scene::init(::Scene &scene)
{
    auto surfaceLists = scene.getSurfaces();
    std::vector<GLfloat> positionsGL;
    std::vector<GLfloat> normalsGL;
    std::vector<GLfloat> colorsGL;
    std::vector<GLuint> indicesGL;

    RandomGenerator random;

    for (auto &surfaces : surfaceLists) {
        for (int i = 0; i < surfaces.size(); i++) {
            int offset = positionsGL.size() / 3;

            auto shape = surfaces[i]->getShape();
            shape->pushVertices(positionsGL);
            shape->pushNormals(normalsGL);
            shape->pushIndices(indicesGL, offset);

            float r = random.next();
            float g = random.next();
            float b = random.next();

            for (int i = 0; i < 3; i++) {
                colorsGL.push_back(r);
                colorsGL.push_back(g);
                colorsGL.push_back(b);
            }
        }
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

        glGenBuffers(1, &mEntityIDs.normalBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, mEntityIDs.normalBufferID);
        glBufferData(
            GL_ARRAY_BUFFER,
            sizeof(GLfloat) * normalsGL.size(),
            (GLvoid *)&normalsGL[0],
            GL_STATIC_DRAW
        );

        glGenBuffers(1, &mEntityIDs.colorBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, mEntityIDs.colorBufferID);
        glBufferData(
            GL_ARRAY_BUFFER,
            sizeof(GLfloat) * colorsGL.size(),
            (GLvoid *)&colorsGL[0],
            GL_STATIC_DRAW
        );
    }

    mTriangleCount = indicesGL.size() / 3;
}

void gl::Scene::draw()
{
    glBindVertexArray(mEntityIDs.vertexArrayID);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, mEntityIDs.vertexBufferID);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, mEntityIDs.normalBufferID);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, mEntityIDs.colorBufferID);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEntityIDs.vertexIndexBufferID);
    glDrawElements(GL_TRIANGLES, mTriangleCount * 3, GL_UNSIGNED_INT, 0);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);
}
