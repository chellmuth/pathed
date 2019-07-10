#pragma once

#include "gl_points.h"
#include "gl_scene.h"
#include "shader.h"

#include "json.hpp"
#include <nanogui/opengl.h>

#include <memory>
#include <string>

namespace gl {
    class PhotonRenderer {
    public:
        PhotonRenderer();

        void init(const std::string &jsonFile);
        void draw(
            GLfloat (&model)[4][4],
            GLfloat (&view)[4][4],
            GLfloat (&projection)[4][4]
        );

        void updateBuffers();
        void updateDebugMode();

    protected:
        std::vector<GLfloat> getPositions();
        std::vector<GLfloat> getColors();

        nlohmann::json m_pointsJson;

        Shader m_shader;
        EntityIDs m_entityIDs;

        DebugMode m_debugMode;

        int m_pointCount;
    };
}
