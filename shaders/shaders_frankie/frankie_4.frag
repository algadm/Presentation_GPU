#version 330 core

in vec3 g_normal;
in vec3 g_pos;
out vec4 color;

void main()
{
    color = vec4(normalize(g_normal) * 0.5 + 1.0, 1.0);
}
