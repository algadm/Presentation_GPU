#version 330 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in vec3 v_pos[];

out vec3 g_normal;
uniform mat4 MVP;

void main()
{
    vec3 p0 = v_pos[0];
    vec3 p1 = v_pos[1];
    vec3 p2 = v_pos[2];
    vec3 normal = normalize(cross(p1 - p0, p2 - p0));

    for (int i = 0; i < 3; i++)
    {
        g_normal = normal;
        gl_Position = MVP * vec4(v_pos[i], 1.0);
        EmitVertex();
    }

    EndPrimitive();
}
