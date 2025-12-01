#include <iostream>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "glad/gl.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "glhelper.h"
#include "camera.h"
#include "mesh.h"

GLuint vao;
GLuint n_elements;

Camera cam;
GLuint program_id;
GLuint program_tbn_id;

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
  program_tbn_id = glhelper::create_program_from_file("./shaders/texture.vert","./shaders/normal.geom", "./shaders/color.frag");
  program_id = glhelper::create_program_from_file("./shaders/texture.vert","./shaders/texture.frag");

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


  Mesh m = Mesh::create_sphere(200, 200);
  n_elements = m.size_element();
  vao = m.load_to_gpu();

  glEnable(GL_DEPTH_TEST);
  glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
}

void display_callback()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  cam.draw_frame();

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
}

void keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  int viewport[4];
  int polygonMode;

  if(action == GLFW_PRESS)
    switch (key)
    {
      case GLFW_KEY_LEFT:
        light_theta += 0.05f;   // rotate left
        break;
      case GLFW_KEY_RIGHT:
        light_theta -= 0.05f;   // rotate right
        break;
      case GLFW_KEY_UP:
        light_phi += 0.05f;     // rotate upward
        // clamp: prevents flipping upside-down
        light_phi = glm::clamp(light_phi, -1.5f, +1.5f);
        break;
      case GLFW_KEY_DOWN:
        light_phi -= 0.05f;     // rotate downward
        light_phi = glm::clamp(light_phi, -1.5f, +1.5f);
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
