#version 330 core

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

out vec3 frag_normal;

void main()
{
    frag_normal = normal;

    gl_Position = projection * view * model * vec4(position, 1.0);
}
