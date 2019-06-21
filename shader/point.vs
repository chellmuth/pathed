#version 330 core

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

out vec3 frag_color;

void main()
{
    frag_color = color;

    gl_Position = projection * view * model * vec4(position, 1.0);
    gl_PointSize = 10.0;
}
