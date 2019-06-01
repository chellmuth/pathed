#define  GL_SILENCE_DEPRECATION 1

#include "shader.h"

#include <fstream>
#include <sstream>


static std::string shaderString(const char *file_path)
{
    std::string shaderString;
	std::ifstream shaderStream(file_path, std::ios::in);
	if (shaderStream.is_open()) {
		std::stringstream sstr;
		sstr << shaderStream.rdbuf();
        shaderString = sstr.str();
		shaderStream.close();
	} else {
		printf("Couldn't find file %s\n", file_path);
    }

    return shaderString;
}

Shader shader::createProgram(const char *vertexShaderPath, const char *fragmentShaderPath) {
    Shader shader;

    GLint Result = GL_FALSE;
    int InfoLogLength;

    shader.vertexID = glCreateShader(GL_VERTEX_SHADER);

    std::string vertexShaderString = shaderString(vertexShaderPath);
	const char *vertexShaderChar = vertexShaderString.c_str();
    glShaderSource(shader.vertexID, 1, &vertexShaderChar, nullptr);
    glCompileShader(shader.vertexID);

    glGetShaderiv(shader.vertexID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(shader.vertexID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
        glGetShaderInfoLog(shader.vertexID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
        printf("%s\n", &VertexShaderErrorMessage[0]);
    }

    shader.fragmentID = glCreateShader(GL_FRAGMENT_SHADER);

    std::string fragmentShaderString = shaderString(fragmentShaderPath);
	const char *fragmentShaderChar = fragmentShaderString.c_str();
    glShaderSource(shader.fragmentID, 1, &fragmentShaderChar, nullptr);
    glCompileShader(shader.fragmentID);

    glGetShaderiv(shader.fragmentID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(shader.fragmentID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
        glGetShaderInfoLog(shader.fragmentID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
        printf("%s\n", &FragmentShaderErrorMessage[0]);
    }

    shader.programID = glCreateProgram();
    glAttachShader(shader.programID, shader.vertexID);
    glAttachShader(shader.programID, shader.fragmentID);

    glLinkProgram(shader.programID);

    glGetProgramiv(shader.programID, GL_LINK_STATUS, &Result);
    glGetProgramiv(shader.programID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> ProgramErrorMessage(InfoLogLength+1);
        glGetProgramInfoLog(shader.programID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
        printf("%s\n", &ProgramErrorMessage[0]);
    }

    return shader;
}
