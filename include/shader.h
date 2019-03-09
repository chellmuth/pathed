#pragma once

#include <nanogui/opengl.h>

typedef struct {
    GLuint vertexID;
    GLuint fragmentID;
    GLuint programID;
} Shader;

namespace shader {
    Shader createProgram(const char *vertexShaderPath, const char *fragmentShaderPath);
}
