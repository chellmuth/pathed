#pragma once

class Ray;

class Camera {
public:
    Camera(float verticalFOV);

    Ray generateRay(int row, int col, int resolutionX, int resolutionY);

private:
    float m_zNear;
    float m_verticalFOV;
};
