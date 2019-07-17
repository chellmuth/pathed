#pragma once

#include "gl_points.h"
#include "gl_scene.h"
#include "gl_types.h"
#include "gl_visualization.h"
#include "kd_tree.h"
#include "point.h"
#include "shader.h"

#include "json.hpp"
#include <nanogui/opengl.h>

#include <memory>
#include <string>
#include <vector>

namespace gl {
    class PhotonRenderer : public gl::Visualization {
    public:
        PhotonRenderer();
        ~PhotonRenderer();

        void init(const std::string &jsonFile);
        void init(
            const Point3 &queryPoint,
            const std::vector<DataSource::Point> &photons,
            DebugMode debugMode
        );
        void draw(
            GLfloat (&model)[4][4],
            GLfloat (&view)[4][4],
            GLfloat (&projection)[4][4]
        ) override;

        void updateBuffers();
        void updateDebugMode();

    protected:
        std::vector<GLfloat> getPositions();
        std::vector<GLfloat> getColors();

        nlohmann::json m_pointsJson;

        Shader m_shader;
        EntityIDs m_entityIDs;

        int m_pointCount;

        Point3 m_queryPoint;
        std::vector<DataSource::Point> m_photons;
        DebugMode m_debugMode;
    };
}
