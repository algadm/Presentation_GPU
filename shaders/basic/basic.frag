#version 330 core

in vec3 new_speed;

out vec4 color;

void main()
{
	color = vec4((normalize(new_speed) + 1.0) / 2.0, 1.0);
}
