// #include <chrono>
// #include <iostream>
// #include <fstream>
// #include <random>
// #include "imgui.h"
// #include "imgui_impl_glfw.h"
// #include "imgui_impl_opengl3.h"

// #include "glad/gl.h"
// #define GLFW_INCLUDE_NONE
// #include <GLFW/glfw3.h>

// #include "glhelper.h"
// #include "camera.h"

// enum
// {
//   POSITION0,
//   POSITION1,
//   VITESSE0,
//   VITESSE1
// };

// const int NB_PARTICULES = 10000;

// GLuint VAO;
// GLuint VBO[4];

// Camera cam;

// GLuint draw_program;
// GLuint tf_program;

// GLuint nframe;
// std::chrono::high_resolution_clock::time_point start;


// void init()
// {
//   draw_program = glhelper::create_program_from_file("shaders/basic.vert", "shaders/basic.frag");

//   std::vector<GLfloat> positions(NB_PARTICULES*3);
//   std::vector<GLfloat> vitesses(NB_PARTICULES*3);

//   std::default_random_engine generator;
//   std::uniform_real_distribution<float> distribution(-0.3,0.3);
//   for(auto i = 0u; i < NB_PARTICULES*3; ++i)
//   {
//     positions[i] = 0.;
//     vitesses[i] = ((i+2)%3) == 0 ? std::fabs(distribution(generator)) : distribution(generator);
//   }
//   std::cout << "Size particules in RAM : " 
//     << float(sizeof(float) * positions.size())/std::mega::num  << " Mbytes" << std::endl;

//   // ########### For Transform Feedbacks ########### //
//   GLuint vs_id = glhelper::compile_shader(glhelper::read_file("shaders/tf.vert").c_str(),GL_VERTEX_SHADER);
//   GLuint fs_id = glhelper::compile_shader(glhelper::read_file("shaders/basic.frag").c_str(),GL_FRAGMENT_SHADER);

//   tf_program = glCreateProgram();
//   glAttachShader(tf_program, vs_id);
//   glAttachShader(tf_program, fs_id);

//   char* var[] = {"pos", "new_speed"};
//   glTransformFeedbackVaryings(tf_program, 2, var, GL_SEPARATE_ATTRIBS);

//   glLinkProgram(tf_program);
//   glhelper::check_error_link(tf_program);
//   glDeleteShader(vs_id);
//   glDeleteShader(fs_id);
//   // ############################################### //

//   glGenVertexArrays(1, &VAO);
//   glBindVertexArray(VAO);

//   glGenBuffers(4, VBO);

//   glBindBuffer(GL_ARRAY_BUFFER, VBO[POSITION0]);
//   glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*positions.size(), positions.data(), GL_DYNAMIC_DRAW);
//   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); 
//   glEnableVertexAttribArray(0); 

//   glBindBuffer(GL_ARRAY_BUFFER, VBO[VITESSE0]);
//   glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*vitesses.size(),vitesses.data(), GL_DYNAMIC_DRAW);
//   glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0); 
//   glEnableVertexAttribArray(1); 

//   glBindBuffer(GL_ARRAY_BUFFER, VBO[POSITION1]);
//   glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*positions.size(), positions.data(), GL_DYNAMIC_DRAW);

//   glBindBuffer(GL_ARRAY_BUFFER, VBO[VITESSE1]);
//   glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*vitesses.size(),vitesses.data(), GL_DYNAMIC_DRAW);

//   glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
//   glPointSize(3);

//   start = std::chrono::high_resolution_clock::now();
// }

// void compute_fps()
// {
//  ++nframe;
//   if(nframe == 100)
//   {
//     auto stop = std::chrono::high_resolution_clock::now();
//     auto diff = (stop-start).count();
//     std::cout << 100./(float(diff) / std::nano::den) << std::endl;
//     nframe = 0;
//     start = stop;
//   }
// }

// void set_uniform_mvp(GLuint program)
// {
//   GLint mvp_id = glGetUniformLocation(program, "MVP");
//   GLint current_prog_id;
//   glGetIntegerv(GL_CURRENT_PROGRAM, &current_prog_id);
//   glUseProgram(program);
//   if (mvp_id != -1)
//   {
//     glm::mat4 model = glm::mat4(1.0f);
//     glm::mat4 mvp = cam.projection()*cam.view()*model;
//     glUniformMatrix4fv(mvp_id, 1, GL_FALSE, &mvp[0][0]);
//   }
//   glUseProgram(current_prog_id);
// }

// void display_callback()
// {
//   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//   // TODO
//   // Disable rasterisation
//   glEnable(GL_RASTERIZER_DISCARD);
//   // Switch to tf program
//   glUseProgram(tf_program);
//   // Use the buffer to fill with the TF information -> glBindBufferBase()
//   glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, VBO[POSITION1]);
//   glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, VBO[VITESSE1]);
//   // Use the good VAO, be sure the pointers to data are valid
//   glBindVertexArray(VAO);
//   // Start the TF and "draw" points
//   glBeginTransformFeedback(GL_POINTS);
//   glDrawArrays(GL_POINTS, 0, NB_PARTICULES);
//   glEndTransformFeedback();
//   // Wait for the buffer to be filled
//   glFlush();
//   // Swap read and write buffers
//   glDisable(GL_RASTERIZER_DISCARD);
//   //END TODO

//   glUseProgram(draw_program);
//   set_uniform_mvp(draw_program);
//   glDrawArrays(GL_POINTS, 0, NB_PARTICULES);

//   // TODO 
//   // Update VAO after TF to use the new VBO with correct pointers
//   std::swap(VBO[POSITION0], VBO[POSITION1]);
//   std::swap(VBO[VITESSE0], VBO[VITESSE1]);

//   glBindVertexArray(VAO);

//   glBindBuffer(GL_ARRAY_BUFFER, VBO[POSITION0]);
//   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); 
//   glEnableVertexAttribArray(0); 

//   glBindBuffer(GL_ARRAY_BUFFER, VBO[VITESSE0]);
//   glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0); 
//   glEnableVertexAttribArray(1); 
//   // END TODO

//   glBindVertexArray(0);
//   compute_fps();
// }

// void keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
// {
//   int viewport[4];

//   if (action == GLFW_PRESS) 
//     switch (key)
//     {
//       case GLFW_KEY_P:
//         glGetIntegerv(GL_VIEWPORT, viewport);
//         glhelper::print_screen(viewport[2], viewport[3]);
//         break;
//       case GLFW_KEY_Q:
//       case GLFW_KEY_ESCAPE:
//         glfwSetWindowShouldClose(window, GLFW_TRUE);
//     }
// }

// void reshape_callback(GLFWwindow* window, int width, int height)
// {
//   cam.common_reshape(width,height);
//   glViewport(0,0, 2*width, 2*height);
// }

// void motion_callback(GLFWwindow* window)
// {
//   double x, y;
//   glfwGetCursorPos(window, &x, &y);
//   auto left = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
//   auto right = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
//   cam.common_motion(x, y, left, right);
// }

// int main(int argc, char** argv)
// {
//   if (!glfwInit()) {
//     std::cerr << "Failed to initialize GLFW" << std::endl;
//     return -1;
//   }

//   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

//   GLFWwindow* window = glfwCreateWindow(cam.width(), cam.height(), "OpenGL", NULL, NULL);
//   if (!window) {
//       std::cerr << "Failed to create GLFW window" << std::endl;
//       glfwTerminate();
//       return -1;
//   }

//   glfwMakeContextCurrent(window);
//   glfwSwapInterval(0);

//   if (!gladLoadGL(glfwGetProcAddress)) {
//       std::cerr << "Failed to initialize GLAD" << std::endl;
//       return -1;
//   }

//   IMGUI_CHECKVERSION();
//   ImGui::CreateContext();
//   ImGuiIO& io = ImGui::GetIO();
//   ImGui_ImplGlfw_InitForOpenGL(window, true);
//   ImGui_ImplOpenGL3_Init();

//   int major, minor;
//   glGetIntegerv(GL_MAJOR_VERSION, &major);
//   glGetIntegerv(GL_MINOR_VERSION, &minor);
//   std::cout << "OpenGL Version: " << major <<"."<< minor << std::endl;

//   glfwSetKeyCallback(window, keyboard_callback);
//   glfwSetWindowSizeCallback(window, reshape_callback);
  
//   init();

//   while (!glfwWindowShouldClose(window)) {
//     glClearColor(0.2f, 0.2f, 0.6f, 1.0f);
//     glClear(GL_COLOR_BUFFER_BIT);

//     ImGui_ImplOpenGL3_NewFrame();
//     ImGui_ImplGlfw_NewFrame();
//     ImGui::NewFrame();

//     ImGui::Begin("Controls");
//     ImGui::Text("Hello, world!");
//     ImGui::End();

//     display_callback();

//     ImGui::Render();
//     ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

//     glfwSwapBuffers(window);

//     motion_callback(window);
//     glfwPollEvents();
//   }

//   ImGui_ImplOpenGL3_Shutdown();
//   ImGui_ImplGlfw_Shutdown();
//   ImGui::DestroyContext();

//   glfwDestroyWindow(window);
//   glfwTerminate();

//   return 0;
// }
