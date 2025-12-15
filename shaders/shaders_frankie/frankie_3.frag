#version 330 core

in vec2 g_texcoord;
out vec4 color;

uniform sampler2D myTextureSampler;

void main()
{
    color = texture(myTextureSampler, g_texcoord);
}
