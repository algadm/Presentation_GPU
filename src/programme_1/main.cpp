#include <chrono>
#include <iostream>
#include <fstream>
#include <random>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "glad/gl.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "glhelper.h"
#include "camera.h"
#include "mesh.h"

enum
{
  POSITION0,
  POSITION1,
  VITESSE0,
  VITESSE1,
};

const int NB_PARTICULES = 100000;

GLuint vao;
GLuint vao_particle;
GLuint VBO[5];
GLuint n_elements;

Camera cam;
GLuint program_id;
GLuint program_tbn_id;
GLuint draw_particles;
GLuint tf_program;

GLuint nframe;

glm::vec3 origin(0.0f, 0.0f, 0.0f);
const float mass = 0.1f;
const float radius = 1.005f;
const float radius_spawn = 2.0f;
const float reflect_coef = 0.70f;
const float friction_coef = 0.80f;

std::chrono::high_resolution_clock::time_point start;

// value for the rotation of the model
bool turn = false;
float angle = 0.0f;

// value to display the TBN basis
bool tbn = false;

float light_theta = 0.0f; // horizontal rotation (left-right)
float light_phi   = 0.0f; // vertical rotation (up-down)


void charge_texture(GLuint& program_id, std::string name, GLuint pos, std::string texture)
{
  glUseProgram(program_id);
  GLuint id = glGetUniformLocation(program_id, name.c_str());
  glUniform1i(id, pos);
  glActiveTexture(GL_TEXTURE0+pos);

  glhelper::load_texture(texture);
}

void init()
{
  program_tbn_id = glhelper::create_program_from_file("shaders/texture/texture.vert","shaders/texture/normal.geom", "shaders/color/color.frag");
  program_id = glhelper::create_program_from_file("shaders/texture/texture.vert","shaders/texture/texture.frag");
  draw_particles = glhelper::create_program_from_file("shaders/basic/basic.vert", "shaders/basic/basic.frag");

  Mesh m = Mesh::create_sphere(200, 200);
  n_elements = m.size_element();
  vao = m.load_to_gpu();

  // Occlusion
  charge_texture(program_id, "myOcclusionSampler", 1, "./data/Rocks002_2K/Rocks002_2K_AmbientOcclusion.png");
  // Color
  charge_texture(program_id, "myTextureSampler", 2, "./data/Rocks002_2K/Rocks002_2K_Color.png");
  // Displacement
  charge_texture(program_id, "myDisplacementSampler", 3, "./data/Rocks002_2K/Rocks002_2K_Displacement.png");
  // Normal
  charge_texture(program_id, "myNormalSampler", 4, "./data/Rocks002_2K/Rocks002_2K_Normal.png");
  // Roughness
  charge_texture(program_id, "myRoughnessSampler", 5, "./data/Rocks002_2K/Rocks002_2K_Roughness.png");

  

  std::vector<GLfloat> positions(NB_PARTICULES*3);
  std::vector<GLfloat> vitesses(NB_PARTICULES*3);

  std::default_random_engine gen;
  std::uniform_real_distribution<float> distAngle(-1.0f, 1.0f);
  for (unsigned i = 0; i < NB_PARTICULES; ++i)
  {
      // ---- 1) Random point on sphere ----
      glm::vec3 p;
      while (true) {
          p = glm::vec3(distAngle(gen), distAngle(gen), distAngle(gen));
          float len2 = glm::dot(p, p);
          if (len2 > 0.001f && len2 <= 1.f) {
              p = glm::normalize(p) * radius_spawn;
              break;
          }
      }

      positions[3*i+0] = p.x;
      positions[3*i+1] = p.y;
      positions[3*i+2] = p.z;

      // ---- 2) Random velocity but biased towards center ----
      glm::vec3 inward = glm::normalize(-p);
      glm::vec3 random = glm::normalize(glm::vec3(distAngle(gen), distAngle(gen), distAngle(gen)));

      // Blend: mostly inward but still with tangential randomness
      glm::vec3 dir = glm::normalize(0.2f*inward + 0.8f*random);

      float speed_mag = 0.5f; // tweak
      glm::vec3 v = dir * speed_mag;

      vitesses[3*i+0] = v.x;
      vitesses[3*i+1] = v.y;
      vitesses[3*i+2] = v.z;
  }
  std::cout << "Size particules in RAM : " 
    << float(sizeof(float) * positions.size())/std::mega::num  << " Mbytes" << std::endl;

  // ########### For Transform Feedbacks ########### //
  GLuint vs_id = glhelper::compile_shader(glhelper::read_file("shaders/tf/tf.vert").c_str(),GL_VERTEX_SHADER);
  GLuint fs_id = glhelper::compile_shader(glhelper::read_file("shaders/basic/basic.frag").c_str(),GL_FRAGMENT_SHADER);

  tf_program = glCreateProgram();
  glAttachShader(tf_program, vs_id);
  glAttachShader(tf_program, fs_id);

  char* var[] = {"pos", "new_speed"};
  glTransformFeedbackVaryings(tf_program, 2, var, GL_SEPARATE_ATTRIBS);

  glLinkProgram(tf_program);
  glhelper::check_error_link(tf_program);
  glDeleteShader(vs_id);
  glDeleteShader(fs_id);

  charge_texture(tf_program, "myDisplacementSampler", 6, "./data/Rocks002_2K/Rocks002_2K_Displacement.png");
  charge_texture(tf_program, "myNormalSampler", 7, "./data/Rocks002_2K/Rocks002_2K_Normal.png");

  glGenVertexArrays(1, &vao_particle);
  glBindVertexArray(vao_particle);
  glGenBuffers(4, VBO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO[POSITION0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*positions.size(), positions.data(), GL_DYNAMIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); 
  glEnableVertexAttribArray(0); 

  glBindBuffer(GL_ARRAY_BUFFER, VBO[VITESSE0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*vitesses.size(),vitesses.data(), GL_DYNAMIC_DRAW);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0); 
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, VBO[POSITION1]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*positions.size(), positions.data(), GL_DYNAMIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, VBO[VITESSE1]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*vitesses.size(),vitesses.data(), GL_DYNAMIC_DRAW);

  glEnable(GL_DEPTH_TEST);
  glClearColor(0.3f, 0.3f, 0.3f, 1.0f);

  start = std::chrono::high_resolution_clock::now();
}

void compute_fps()
{
 ++nframe;
  if(nframe == 100)
  {
    auto stop = std::chrono::high_resolution_clock::now();
    auto diff = (stop-start).count();
    std::cout << 100./(float(diff) / std::nano::den) << std::endl;
    nframe = 0;
    start = stop;
  }
}

void set_uniform_mvp(GLuint program)
{
  GLint mvp_id = glGetUniformLocation(program, "MVP");
  GLint current_prog_id;
  glGetIntegerv(GL_CURRENT_PROGRAM, &current_prog_id);
  glUseProgram(program);
  if (mvp_id != -1)
  {
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 mvp = cam.projection()*cam.view()*model;
    glUniformMatrix4fv(mvp_id, 1, GL_FALSE, &mvp[0][0]);
  }
  glUseProgram(current_prog_id);
}

void display_callback()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  cam.draw_frame();
  // Disable rasterisation
  glEnable(GL_RASTERIZER_DISCARD);
  // Switch to tf program
  glUseProgram(tf_program);
  glUniform3fv(glGetUniformLocation(tf_program,"origin"),1,&origin[0]);
  glUniform1f(glGetUniformLocation(tf_program,"mass"), mass);
  glUniform1f(glGetUniformLocation(tf_program,"radius"), radius);
  glUniform1f(glGetUniformLocation(tf_program,"reflect_coef"), reflect_coef);
  glUniform1f(glGetUniformLocation(tf_program,"friction_coef"), friction_coef);

  // Use the buffer to fill with the TF information -> glBindBufferBase()
  glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, VBO[POSITION1]);
  glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, VBO[VITESSE1]);
  // Use the good VAO, be sure the pointers to data are valid
  glBindVertexArray(vao_particle);
  // Start the TF and "draw" points
  glBeginTransformFeedback(GL_POINTS);
  glDrawArrays(GL_POINTS, 0, NB_PARTICULES);
  glEndTransformFeedback();
  // Wait for the buffer to be filled
  glFlush();
  // Swap read and write buffers
  glDisable(GL_RASTERIZER_DISCARD);
  //END TODO

  glUseProgram(draw_particles);
  set_uniform_mvp(draw_particles);
  glDrawArrays(GL_POINTS, 0, NB_PARTICULES);
  // Update VAO after TF to use the new VBO with correct pointers
  std::swap(VBO[POSITION0], VBO[POSITION1]);
  std::swap(VBO[VITESSE0], VBO[VITESSE1]);

  glBindVertexArray(vao_particle);

  glBindBuffer(GL_ARRAY_BUFFER, VBO[POSITION0]);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); 
  glEnableVertexAttribArray(0); 

  glBindBuffer(GL_ARRAY_BUFFER, VBO[VITESSE0]);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0); 
  glEnableVertexAttribArray(1);
  // END TODO

  glUseProgram(program_id);
  glm::mat4 model = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));
  glhelper::set_uniform_mat4(program_id, "Model", model);
  glhelper::set_uniform_mat4(program_id, "View", cam.view());
  glhelper::set_uniform_mat4(program_id, "Perspective", cam.projection());

  glm::vec3 light_pos = glhelper::compute_light_position(light_theta, light_phi);
  glhelper::set_uniform_vec3(program_id, "LightPos", light_pos);
  glBindVertexArray(vao);
  glDrawElements(GL_TRIANGLES, n_elements, GL_UNSIGNED_INT, 0);

  if(tbn)
  {
    glUseProgram(program_tbn_id);
    glhelper::set_uniform_mat4(program_tbn_id, "Model", model);
    glhelper::set_uniform_mat4(program_tbn_id, "View", cam.view());
    glhelper::set_uniform_mat4(program_tbn_id, "Perspective", cam.projection());
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, n_elements, GL_UNSIGNED_INT, 0);
  }

  CHECK_GL_ERROR();

  glBindVertexArray(0);
  compute_fps();
}

void keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  int viewport[4];
  int polygonMode;

  if(action == GLFW_PRESS)
    switch (key)
    {
      case GLFW_KEY_LEFT:
        light_theta += 0.1f;   // rotate left
        break;
      case GLFW_KEY_RIGHT:
        light_theta -= 0.1f;   // rotate right
        break;
      case GLFW_KEY_UP:
        light_phi += 0.1f;     // rotate upward
        break;
      case GLFW_KEY_DOWN:
        light_phi -= 0.1f;     // rotate downward
        break;
      case GLFW_KEY_O:
        turn = !turn;
    break;
      case GLFW_KEY_F:
        tbn = !tbn;
      break;
      case GLFW_KEY_W:
        glGetIntegerv(GL_POLYGON_MODE, &polygonMode);
        glPolygonMode(GL_FRONT_AND_BACK, polygonMode == GL_LINE ? GL_FILL : GL_LINE);
    break;
      case GLFW_KEY_P:
        glGetIntegerv(GL_VIEWPORT, viewport);
        glhelper::print_screen(viewport[2], viewport[3]);
        break;
      case GLFW_KEY_Q:
      case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        break;
    }
}

void reshape_callback(GLFWwindow* window, int width, int height)
{
  cam.common_reshape(width,height);
  glViewport(0,0, 2*width, 2*height);
}

void motion_callback(GLFWwindow* window)
{
  double x, y;
  glfwGetCursorPos(window, &x, &y);
  auto left = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
  auto right = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
  cam.common_motion(x, y, left, right);
}

int main(int argc, char** argv)
{
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW" << std::endl;
    return -1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow* window = glfwCreateWindow(cam.width(), cam.height(), "OpenGL", NULL, NULL);
  if (!window) {
      std::cerr << "Failed to create GLFW window" << std::endl;
      glfwTerminate();
      return -1;
  }

  glfwMakeContextCurrent(window);
  glfwSwapInterval(0);

  if (!gladLoadGL(glfwGetProcAddress)) {
      std::cerr << "Failed to initialize GLAD" << std::endl;
      return -1;
  }

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init();
  int major, minor;
  glGetIntegerv(GL_MAJOR_VERSION, &major);
  glGetIntegerv(GL_MINOR_VERSION, &minor);
  std::cout << "OpenGL Version: " << major <<"."<< minor << std::endl;

  glfwSetKeyCallback(window, keyboard_callback);
  glfwSetWindowSizeCallback(window, reshape_callback);

  init();
  double prev_time = glfwGetTime();
  while (!glfwWindowShouldClose(window)) {

    double current_time = glfwGetTime();
    double delta_time = current_time - prev_time;
    prev_time = current_time;

    glClearColor(0.2f, 0.2f, 0.6f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Controls");
    ImGui::Text("'o' to toggle rotation");
    ImGui::Text("'f' to toggle TBN display");
    ImGui::Text("'w' to toggle wireframe");
    ImGui::Text("'p' to print screen");
    ImGui::Text("'q' or esc to quit");
    ImGui::End();

    if(turn) angle += 3.14/5 * delta_time;
    display_callback();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);

    motion_callback(window);
    glfwPollEvents();
  }

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
