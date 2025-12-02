#version 330 core

layout(triangles) in;
layout(line_strip, max_vertices = 18) out;

out vec3 c;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Perspective;

in VertexData
{
    vec3 position_TBN;
    vec3 position_view;
    vec3 normal_view;
    vec3 tangent_view;
    vec3 bitangent_view;
    vec2 texCoord;
    vec3 light_TBN;
} vsData[];

void main(void)
{
    for( int i = 0; i < 3; ++i)
    {
        gl_Position = Perspective * vec4(vsData[i].position_view, 1.0);
        c = vec3(1.,0.,0.);
        EmitVertex();
        gl_Position = Perspective * vec4(vsData[i].position_view + 0.1*vsData[i].normal_view, 1.0);
        c = vec3(1.,0.,0.);
        EmitVertex();
        EndPrimitive();
        gl_Position = Perspective * vec4(vsData[i].position_view, 1.0);
        c = vec3(0.,1.,0.);
        EmitVertex();
        gl_Position = Perspective * vec4(vsData[i].position_view + 0.1*vsData[i].tangent_view, 1.0);
        c = vec3(0.,1.,0.);
        EmitVertex();
        EndPrimitive();
        gl_Position = Perspective * vec4(vsData[i].position_view, 1.0);
        c = vec3(0.,0.,1.);
        EmitVertex();
        gl_Position = Perspective * vec4(vsData[i].position_view + 0.1*vsData[i].bitangent_view, 1.0);
        c = vec3(0.,0.,1.);
        EmitVertex();
        EndPrimitive();
    }
}