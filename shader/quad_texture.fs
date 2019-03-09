#version 330 core

out vec4 out_color;

uniform sampler2D sampler;

void main()
{
    float s = 0.5*gl_FragCoord.x/1024;
    float t = 0.5*gl_FragCoord.y/1024;

    out_color = vec4(texture(sampler, vec2(s, t)).rgb, 1.0);
}
