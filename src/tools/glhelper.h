#ifndef GLHELPER_H
#define GLHELPER_H

#include "glad/gl.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <ctime>

#include "glm/glm.hpp"


//https://blog.nobel-joergensen.com/2013/01/29/debugging-opengl-using-glgeterror/
void _check_gl_error(const char *file, int line);
#define CHECK_GL_ERROR() _check_gl_error(__FILE__,__LINE__)

namespace glhelper
{
  // Verify if error free while linking program
  void check_error_link(GLuint program_id);

  // return id of the gpu program
  GLuint create_program_from_file(const std::string& vs_file,
      const std::string& fs_file);

  GLuint create_program_from_file(const std::string& vs_file,
      const std::string& tcs_file,
      const std::string& tes_file,
      const std::string& fs_file);

  // return id of the gpu program
  GLuint create_program_from_file(const std::string& vs_file,
      const std::string& gs_file,
      const std::string& fs_file);

  // return id of the gpu program
  GLuint create_program(const std::string& vs_content,
      const std::string& fs_content);

  GLuint create_program(const std::string& vs_file,
      const std::string& tcs_file,
      const std::string& tes_file,
      const std::string& fs_file);

  // return id of the gpu program
  GLuint create_program(const std::string& vs_content,
      const std::string& gs_content,
      const std::string& fs_content);

  // return id of the compiled shader
  GLuint compile_shader(const char* shader_content, GLenum shader_type);

  // return content of file filename
  std::string read_file(const std::string& filename);

  // set uniform value of mat4 by name 
  void set_uniform_mat4(GLuint program, std::string name, glm::mat4 mat);

  void set_uniform_vec3(GLuint program, std::string name, glm::vec3 light_pose);

  glm::vec3 compute_light_position(const float light_theta, const float light_phi);

  // Creation texture et envoie sur GPU
  GLuint load_texture(std::string filename);

  // Creation d'une image
  void load_image(const std::string& filename,
      unsigned char*& image,
      int& w,
      int& h);

  void print_screen(int w, int h, std::string filename = "");

} // namespace glhelper

#endif
