#include "mesh.h"
#include <iostream> 

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/component_wise.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/constants.hpp"

Mesh::Mesh(const std::vector<GLfloat>& v, const std::vector<GLuint>& i)
  : vertices(v), indices(i){}

Mesh Mesh::load_from_file(const std::string filename)
{
  std::string warn, err;

  std::vector<GLfloat> vertices;
  std::vector<GLuint> indices;

  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  vertices.clear();
  indices.clear();

  std::vector<glm::ivec3> index;

  if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.c_str()))
  {
    std::cerr << (warn + err) << std::endl;
  }
  for (const auto& shape : shapes)
    for(const auto& indice : shape.mesh.indices)
    {
      glm::ivec3 ind = glm::ivec3(indice.vertex_index, indice.normal_index, indice.texcoord_index);
      auto it = index.begin();
      while ( it != index.cend() && (*it != ind) ){++it;}
      if(it == index.cend())
      {
        index.emplace_back(indice.vertex_index, indice.normal_index, indice.texcoord_index);
        vertices.push_back(attrib.vertices[3*indice.vertex_index]);
        vertices.push_back(attrib.vertices[3*indice.vertex_index+1]);
        vertices.push_back(attrib.vertices[3*indice.vertex_index+2]);

        if(indice.normal_index != -1)
        {
          vertices.push_back(attrib.normals[3*indice.normal_index]);
          vertices.push_back(attrib.normals[3*indice.normal_index+1]);
          vertices.push_back(attrib.normals[3*indice.normal_index+2]);
        }
        else
        {
          vertices.push_back(1.);
          vertices.push_back(0.);
          vertices.push_back(0.);
        }
        vertices.push_back(0.0); //tangent x
        vertices.push_back(0.0); //tangent y
        vertices.push_back(0.0); //tangent z
        vertices.push_back(0.0); //tangent orientation
        vertices.push_back(attrib.texcoords[2*indice.texcoord_index]);
        vertices.push_back(attrib.texcoords[2*indice.texcoord_index+1]);
      }
    }
  for (const auto& shape : shapes)
    for(const auto& indice : shape.mesh.indices)
    {
      glm::ivec3 ind = glm::ivec3(indice.vertex_index, indice.normal_index, indice.texcoord_index);

      auto it = index.cbegin();
      while ( it != index.cend() && (*it != ind) ){++it;}
      if(it != index.cend())
      {
        indices.push_back(std::distance(index.cbegin(),it));
      }
    }

  return {vertices, indices};
}
unsigned int  Mesh::size_element() const {return indices.size();}

void Mesh::compute_normales()
{
  for(auto i = 0u; i < size_element(); i+= 3)
  {
    auto p0 = glm::vec3(vertices[12*indices[i]], vertices[12*indices[i]+1],vertices[12*indices[i]+2]);
    auto p1 = glm::vec3(vertices[12*indices[i+1]], vertices[12*indices[i+1]+1],vertices[12*indices[i+1]+2]);
    auto p2 = glm::vec3(vertices[12*indices[i+2]], vertices[12*indices[i+2]+1],vertices[12*indices[i+2]+2]);

    auto n = glm::normalize(glm::cross(p1-p0, p2-p0));

    for(auto k = 0u; k < 3; ++k)
      for(auto j = 0u; j < 3; ++j)
        vertices[12*indices[i+k]+3+j] = n[j];
  }
  for(auto i = 0u; i < vertices.size(); i+=12)
  {
    auto n = glm::vec3(vertices[i+3], vertices[i+4],vertices[i+5]);
    n = glm::normalize(n);
    for(auto j = 0u; j < 3; ++j)
      vertices[i+3+j] = n[j];
  }
}

void Mesh::compute_tangant_space()
{
  for(auto i = 0u; i < vertices.size(); i+=12)
  {
    vertices[i+6] = 0.;
    vertices[i+7] = 0.;
    vertices[i+8] = 0.;
    vertices[i+9] = 0.;
  }

  std::vector<glm::vec3> tangant(vertices.size()/12);
  std::vector<glm::vec3> bitangant(vertices.size()/12);

  for(auto i = 0u; i < size_element(); i+= 3)
  {
    auto p0 = glm::vec3(vertices[12*indices[i]],vertices[12*indices[i]+1],vertices[12*indices[i]+2] );
    auto p1 = glm::vec3(vertices[12*indices[i+1]],vertices[12*indices[i+1]+1],vertices[12*indices[i+1]+2] );
    auto p2 = glm::vec3(vertices[12*indices[i+2]],vertices[12*indices[i+2]+1],vertices[12*indices[i+2]+2] );

    auto uv0 = glm::vec2(vertices[12*indices[i]+10],vertices[12*indices[i]+11]);
    auto uv1 = glm::vec2(vertices[12*indices[i+1]+10],vertices[12*indices[i+1]+11]);
    auto uv2 = glm::vec2(vertices[12*indices[i+2]+10],vertices[12*indices[i+2]+11]);

    auto e01 = p1 - p0;
    auto e02 = p2 - p0;

    auto d1 = uv1 - uv0;
    auto d2 = uv2 - uv0;

    auto f = 1.0f / (d1.x*d2.y - d2.x*d1.y);
    auto t = f * (d2.y*e01 - d1.y * e02);
    auto b = f * (d1.x*e02 - d2.x * e01);

    tangant[indices[i]] += t;
    tangant[indices[i+1]] += t;
    tangant[indices[i+2]] += t;

    bitangant[indices[i]] += b;
    bitangant[indices[i+1]] += b;
    bitangant[indices[i+2]] += b;
  }
  for(auto i = 0u; i < vertices.size(); i+=12)
  {
    const auto& t = tangant[i/12];
    const auto& b = bitangant[i/12];
    const auto n = glm::vec3(vertices[i+3], vertices[i+4],vertices[i+5]);
    auto tortho = glm::normalize(t - glm::dot(t, n) * n);
    auto dir = (glm::dot(glm::cross(t, b), n) > 0.) ? 1. : -1.;
    vertices[i+6] = tortho.x;
    vertices[i+7] = tortho.y;
    vertices[i+8] = tortho.z;
    vertices[i+9] = dir;

  }
}

GLuint Mesh::create_VBO() const
{
  GLuint vbo;
  glGenBuffers(1, &vbo);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(GLfloat), 0);

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 12 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));

  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 12 * sizeof(GLfloat), (void*)(10 * sizeof(GLfloat)));

  return vbo;
}
GLuint Mesh::create_EBO() const
{
  GLuint ebo;
  glGenBuffers(1, &ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
  return ebo;
}
GLuint Mesh::load_to_gpu() const
{
  GLuint vao, vbo, ebo;

  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  vbo = create_VBO();
  ebo = create_EBO();

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  return vao;
}

Mesh Mesh::create_grid(int N)
{
  std::vector<GLfloat> vertices;
  std::vector<GLuint> indices;

  GLfloat delta = 1./(float(N)-1.);

  for(int i = 0; i < N; ++i)
  {
    for(int j = 0; j < N; ++j)
    {
      vertices.push_back(-1.+2.*j*delta); // x
      vertices.push_back(0.);// y
      vertices.push_back(-1.+2.*i*delta); // z
      vertices.push_back(0.); // nx
      vertices.push_back(-1.); // ny
      vertices.push_back(0.); // nz
      vertices.push_back(0.); // tx
      vertices.push_back(0.); // ty
      vertices.push_back(0.); // tz
      vertices.push_back(0.); // torientation
      vertices.push_back(j*delta); // u
      vertices.push_back(i*delta); // v
    }
  }
  for(int i = 0; i < N*(N-1); ++i)
  {
    if( (i+1)%N != 0 ) 
    {
      indices.push_back(i);
      indices.push_back(i+1);
      indices.push_back(i+N);

      indices.push_back(i+1);
      indices.push_back(i+N);
      indices.push_back(i+N+1);
    }
  }

  Mesh m(vertices, indices);
  m.compute_tangant_space();
  return m;
}

Mesh Mesh::create_sphere(int slices, int stacks)
{
  std::vector<GLfloat> vertices;
  std::vector<GLuint> indices;

  const float pi = glm::pi<float>();

  const GLfloat du = 2.0f*pi/float(slices-1.0f);
  const GLfloat dv = pi/float(stacks-1.0f);

  GLfloat v = -pi/2.0f;

  for(int j = 0; j < stacks; ++j, v+=dv)
  {  
    GLfloat u = -pi;
    for(int i = 0; i < slices; ++i, u+=du)
    {
      vertices.push_back(cosf(u)*cosf(v)); //x
      vertices.push_back(sinf(v)); //y
      vertices.push_back(sinf(u)*cosf(v)); //z
      vertices.push_back(cosf(u)*cosf(v)); //nx
      vertices.push_back(sinf(v)); //ny
      vertices.push_back(sinf(u)*cosf(v)); //nz
      vertices.push_back(0.); // tx
      vertices.push_back(0.); // ty
      vertices.push_back(0.); // tz
      vertices.push_back(0.); // torientation
      vertices.push_back(1.0f-float(i)/float(slices-1)); //u
      vertices.push_back(float(j)/float(stacks-1)); //v
    }
  }
  for(int j = 0; j < stacks - 1; ++j)
    for(int i = 0; i < slices - 1; ++i)
    {
      indices.push_back(i + j*slices);
      indices.push_back(i + 1 + j*slices);
      indices.push_back(i + (j+1)*slices);

      indices.push_back(i + 1 + j*slices);
      indices.push_back(i + 1 + (j+1)*slices);
      indices.push_back(i + (j+1)*slices);
    }

  Mesh m(vertices, indices);
  m.compute_tangant_space();
  return m;
}

void Mesh::apply_matrix(const glm::mat4& m)
{
  for (auto i = 0u; i < vertices.size(); i+=12)
  {
    glm::vec4 p(vertices[i], vertices[i+1], vertices[i+2], 1.);
    glm::vec4 n(vertices[i+3], vertices[i+4], vertices[i+5], 0.);
    p = m * p;
    n = glm::transpose(glm::inverse(m)) * n;
    vertices[i] = p.x;
    vertices[i+1] = p.y;
    vertices[i+2] = p.z;
    vertices[i+3] = n.x;
    vertices[i+4] = n.y;
    vertices[i+5] = n.z;
  }
  // could be done differently
  compute_tangant_space();
}

 void Mesh::normalize()
{
  auto min = glm::vec3(std::numeric_limits<float>::max());
  auto max = glm::vec3(std::numeric_limits<float>::lowest());

  for (auto i = 0u; i < vertices.size(); i += 12u) {
    glm::vec3 p(vertices[i], vertices[i + 1], vertices[i + 2]);
    min = glm::min(p, min);
    max = glm::max(p, max);
  }
  auto center = (min + max) * 0.5f;
  auto d = max - min;
  auto scale = 1. / glm::compMax(d);
  glm::mat4 m =
      glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(scale)), -center);
  apply_matrix(m);
}

std::vector<GLfloat> Mesh::position() const { 
  std::vector<GLfloat> v;
  for(int i= 0; i+7 < vertices.size(); i+=12)
  {
    v.push_back(vertices[i]);
    v.push_back(vertices[i+1]);
    v.push_back(vertices[i+2]);
  }
  return v;
}
std::vector<GLfloat> Mesh::normal() const { 
  std::vector<GLfloat> v;
  for(int i= 0; i+7 < vertices.size(); i+=12)
  {
    v.push_back(vertices[i+3]);
    v.push_back(vertices[i+4]);
    v.push_back(vertices[i+5]);
  }
  return v;
}
std::vector<GLfloat> Mesh::texcoord() const { 
  std::vector<GLfloat> v;
  for(int i= 0; i+7 < vertices.size(); i+=12)
  {
    v.push_back(vertices[i+6]);
    v.push_back(vertices[i+7]);
  }
  return v;
}

