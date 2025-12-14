#version 330 core

in VertexData
{
    vec3 position_TBN;
    vec3 position_view;
    vec3 normal_view;
    vec3 tangent_view;
    vec3 bitangent_view;
    vec2 texCoord;
    vec3 light_TBN;
} vsData;

out vec4 color;

uniform sampler2D myOcclusionSampler;
uniform sampler2D myTextureSampler;

uniform sampler2D myNormalSampler;
uniform sampler2D myRoughnessSampler;

// n is the normal (should be normalized), 
// light_dir the direction TO the light (should be normalized),
// cam_dir the direction TO the camera (should be normalized),
// color_mat the color of the object for the current fragment,
// color_light the color of the light 
vec3 phong(vec3 n, vec3 light_dir, vec3 cam_dir, vec3 color_mat, vec3 color_light, 
           float ka, float kd, float ks, int shininess, float occlusion)
{
  n = normalize(n);
  float ambiant = ka * occlusion;
  float diffuse = kd * clamp(dot(n, light_dir), 0.0, 1.0) * occlusion;
  float specular = ks * pow(clamp(dot(cam_dir, reflect(-light_dir, n)), 0.0, 1.0), shininess);
  return (ambiant + diffuse) * color_mat + specular * color_light;
}

void main()
{

  float ka = 0.2;
  float kd = 0.8;
  float ks = 0.3;

  vec3 color_mat = texture(myTextureSampler, vsData.texCoord).xyz;
  float color_occ = texture(myOcclusionSampler, vsData.texCoord).r;
  vec3 normal_mat = texture(myNormalSampler, vsData.texCoord).xyz;
  normal_mat = (normal_mat * 2) - 1;
  float roughness = texture(myRoughnessSampler, vsData.texCoord).r;
  float shininess = 2 / (roughness*roughness) - 2;

    // Use everything in camera space
  color = vec4(phong(normalize(normal_mat), normalize(vsData.light_TBN), -normalize(vsData.position_TBN),
   color_mat, vec3(1.0, 1.0, 1.0), ka, kd, ks, int(shininess), color_occ), 1.0);
}