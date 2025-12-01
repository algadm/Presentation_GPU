#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec4 tangent;
layout (location = 3) in vec2 texcoord;

out VertexData
{
    vec3 position_TBN; //position in TBN space
    vec3 position_view; //position in camera space
    vec3 normal_view; //normal in camera space
    vec3 tangent_view; //tangent in camera space
    vec3 bitangent_view; //bitangent in camera space
    vec2 texCoord; //texture coordinate
    vec3 light_TBN; // light direction in TBN space
} vsData;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Perspective;
uniform sampler2D myDisplacementSampler;
uniform vec3 LightPos;

// vec3 light_pos = vec3(10.0, 20.0, 0.0);

void main()
{
    float displacement = texture(myDisplacementSampler, vsData.texCoord).r;
    vec3 my_pos = position + normal * displacement * 0.2;
    gl_Position = Perspective * View * Model * vec4(my_pos, 1.0);

    // Send all information in camera space
    vsData.position_view = (View * Model * vec4(my_pos, 1.0)).xyz;
    // For explanation https://www.lighthouse3d.com/tutorials/glsl-12-tutorial/the-normal-matrix/
    vsData.normal_view = (transpose(inverse(View * Model)) * vec4(normal, 0.0)).xyz;
    vsData.tangent_view = normalize((View * Model * vec4(tangent.xyz, 0.0)).xyz);
    vsData.bitangent_view = normalize(cross(vsData.tangent_view, vsData.normal_view)*tangent.w);

    vsData.texCoord = texcoord;

    vec3 light_view = normalize((View * vec4(LightPos, 1.0)).xyz - vsData.position_view);

    vec3 b = normalize(cross(vsData.tangent_view, vsData.normal_view)*tangent.w);
    // Matrice de passage du repere view au repere tangant
    mat3 TBN = transpose(mat3(vsData.tangent_view, b, vsData.normal_view ));

    vsData.position_TBN = TBN * vsData.position_view;
    vsData.light_TBN = TBN * light_view;
}