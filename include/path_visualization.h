#pragma once

#include "gl_lines.h"
#include "gl_visualization.h"
#include "kd_tree.h"
#include "photon_renderer.h"
#include "sample.h"

namespace gl {
    class PathVisualization : public gl::Visualization {
    public:
        ~PathVisualization() {};
        void init(
            const Sample& sample,
            const std::vector<DataSource::Point> &photons,
            const DataSource &dataSource,
            DebugMode debugMode
        );

        void draw(
            GLfloat (&model)[4][4],
            GLfloat (&view)[4][4],
            GLfloat (&projection)[4][4]
        ) override;

    private:
        PhotonRenderer m_photonRenderer;
        gl::Lines m_glLines;
    };
};
