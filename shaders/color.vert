#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Perspective;

out vec3 c; 

void main()
{
    gl_Position = Perspective * View * Model * vec4(position, 1.0);
    c = color;
}
