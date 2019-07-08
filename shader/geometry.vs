#version 330 core

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 color;

out vec3 frag_normal;
out vec3 frag_color;

void main()
{
    frag_normal = normal;
    frag_color = color;

    gl_Position = projection * view * model * vec4(position, 1.0);
}
