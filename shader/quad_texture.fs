#version 330 core

in vec2 frag_uv;

out vec4 out_color;

uniform sampler2D sampler;

void main()
{
    out_color = vec4(texture(sampler, frag_uv).rgb, 1.0);
}
