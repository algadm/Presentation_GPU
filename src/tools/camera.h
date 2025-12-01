#ifndef CAMERA_H
#define CAMERA_H

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/string_cast.hpp"

#include <algorithm>
#include <iostream>

class Camera
{
  private:
    glm::vec3 pos;
    glm::vec3 target;
    glm::vec3 up;
    float distance;
    float pitch; 
    float heading;


    glm::vec2 origin_mouse;

    int screen_width;
    int screen_height;

  public:
    Camera() ;
    Camera(const Camera&) = default;
    Camera(Camera &&) = default;
    glm::mat4 projection() const;
    glm::mat4 view() const;

    void common_reshape(int w, int h);
    void common_motion(int x, int y, int left, int right);

    int height() const;
    int width() const;

    glm::vec3 position() const;
    glm::vec3 lookat() const;

    void draw_frame() const;
};

#endif
