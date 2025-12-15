#version 330 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

uniform mat4 MVP;
uniform vec3 move;
uniform vec3 frankie_cam_pos;

in vec3 v_pos[];
in vec2 v_texcoord[];
in vec3 v_normal[];

out vec2 g_texcoord;


void main(void)
{
for (int i = 0; i < 3; i++)
	{
	  vec3 normal = normalize(cross(v_pos[1] - v_pos[0], v_pos[2] - v_pos[0]));
	  vec3 moved_pos = v_pos[i] + move;

	  // vec3 cam = vec3(4.500000, 0.000000, 0.000000);

	  if (dot(normal, frankie_cam_pos) < 0)
	  {
		gl_Position = MVP * vec4(moved_pos, 1.0);
		g_texcoord = v_texcoord[i];
		EmitVertex();
	  }
	}
	EndPrimitive();
}