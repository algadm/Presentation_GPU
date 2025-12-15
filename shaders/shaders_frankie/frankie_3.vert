#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 3) in vec2 texcoord;

out vec3 v_pos;
out vec2 v_texcoord;
out vec3 v_normal;

uniform mat4 MVP;
uniform float t;

mat3 rotationY(float angle)
{
    float c = cos(angle);
    float s = sin(angle);

    return mat3(
        c,  0.0, s,
        0.0, 1.0, 0.0,
        -s, 0.0, c
    );
}

void main()
{
    float radius = 2.0;
    float alpha = t;
    float beta = t;

    vec3 orbitOffset = vec3(radius * cos(alpha), 0.0, radius * sin(alpha));
    vec3 rotatedPosition = rotationY(beta) * position;
    rotatedPosition.y *= -1.0;

    v_pos = orbitOffset + rotatedPosition;
    v_texcoord = texcoord;
    v_normal = normal;

    gl_Position = MVP * vec4(v_pos, 1.0);
}
