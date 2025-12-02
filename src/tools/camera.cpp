#include "camera.h"
#include "glhelper.h"
#include <GLFW/glfw3.h>

Camera::Camera()
  : pos(3.,-1.5,3.), 
  target(0.,0.,0.), 
  up(0,1,0),
  distance(glm::distance(pos, target)),
  screen_width(800), 
  screen_height(800) 
{}


int Camera::height() const {return screen_height;}
int Camera::width() const {return screen_width;}

glm::mat4 Camera::projection() const
{
  return glm::perspective(glm::radians(45.0f), (float)screen_width / (float)screen_height, 0.1f, 100.0f);
}

glm::mat4 Camera::view() const
{
  const glm::vec3 new_pos(cos(heading)*cos(pitch),sin(pitch), -sin(heading)*cos(pitch));
  return glm::lookAt(
      glm::normalize((new_pos * pos)-target) * distance + target,
      target,
      up); 
}

void Camera::common_reshape(int w, int h)
{
  screen_width  = w;
  screen_height = h;
}

void Camera::common_motion(int x, int y, int left, int right)
{

  if (left == GLFW_PRESS)
  { 
    pitch += float(origin_mouse.y - y) / (float)screen_height * 5.;
    heading += float(origin_mouse.x - x) / (float)screen_width * 5.;

    pitch = pitch > 1.5 ? 1.5 : pitch < -1.5 ? -1.5 : pitch;
    heading = glm::mod(heading + 3.14f, 6.24f) - 3.14;
  }
  if (right == GLFW_PRESS )
  {
    distance += float (origin_mouse.y - y) * 0.2;
    distance = distance < 1. ? 1. : distance > 20. ? 20. : distance;
  }

  origin_mouse = glm::vec2(x, y);
}

glm::vec3 Camera::position() const 
{
  const glm::vec3 new_pos(cos(heading)*cos(pitch),sin(pitch), -sin(heading)*cos(pitch));
  return glm::normalize((new_pos)-target) * distance + target;
}

glm::vec3 Camera::lookat() const 
{
  return target;
}

static GLuint prog_frame = -1;
static GLuint vao_frame = -1;

void Camera::draw_frame() const
{
  if(prog_frame == -1)
  {
    prog_frame = glhelper::create_program_from_file("./shaders/color/color.vert","./shaders/color/color.frag");
  }
  if(vao_frame == -1)
  {
    float vertices[] = {
      0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,    
      1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
      0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f };
    glGenVertexArrays(1, &vao_frame);
    glBindVertexArray(vao_frame);
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
  }
  
  GLint current_prog_id;
  glGetIntegerv(GL_CURRENT_PROGRAM, &current_prog_id);

  glUseProgram(prog_frame);
  glhelper::set_uniform_mat4(prog_frame, "Model", glm::mat4(1.0f));
  glhelper::set_uniform_mat4(prog_frame, "View", view());
  glhelper::set_uniform_mat4(prog_frame, "Perspective", projection());
  glBindVertexArray(vao_frame);
  glDrawArrays(GL_LINES, 0, 6);

  glUseProgram(current_prog_id);
}
