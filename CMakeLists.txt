cmake_minimum_required(VERSION 3.15)
project(gpu_gpgpu LANGUAGES C CXX)

include(FetchContent)

# =====================
# glfw
# =====================
find_package(glfw3 3.4 QUIET)
if (NOT glfw3_FOUND)
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG master
    )
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(glfw)
endif()

# =====================
# imgui
# =====================
find_package(imgui QUIET)
if (NOT imgui_FOUND)
    FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG master
    )
    FetchContent_MakeAvailable(imgui)

    add_library(imgui
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
    )

    add_library(imgui_backend
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    )

    target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})
    target_include_directories(imgui_backend PUBLIC
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
    )

    target_link_libraries(imgui_backend PUBLIC glfw)
endif()


# =====================
# glm
# =====================
# find_package(glm QUIET)
if (NOT glm_FOUND)
    FetchContent_Declare(
      glm
      GIT_REPOSITORY	https://github.com/g-truc/glm.git
      GIT_TAG 	0af55ccecd98d4e5a8d1fad7de25ba429d60e863 #refs/tags/1.0.1
    )
    FetchContent_MakeAvailable(glm)
endif()

# =====================
# program 1 to 2
# =====================

# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_party/glad/gl33/include)
# target_include_directories(programme_1 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/glad/gl33/include)

message("Building tools library")

file(GLOB_RECURSE source_files "src/tools/*.cpp")
file(GLOB_RECURSE header_files "src/tools/*.h" "src/tools/*.hpp")

message("Source files")
foreach(line IN LISTS source_files)
message("  ${line}")
endforeach()

message("Header files")
foreach(line IN LISTS header_files)
message("  ${line}")
endforeach()

add_library(tools ${source_files} ${header_files})
target_link_libraries(tools PRIVATE glfw glm::glm)
target_include_directories(tools PUBLIC 
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/glad/gl43/include
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/stb/include
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/tinyobjloader/include
  )

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/tools)


foreach(dir_n RANGE 1 1)
  set(dir "programme_${dir_n}")
  
  message("Programme ${dir}" )

  file(GLOB_RECURSE source_files "src/${dir}/*.cpp")
  file(GLOB_RECURSE header_files "src/${dir}/*.h" "src/${dir}/*.hpp") 
  file(GLOB_RECURSE shader_files "shaders/*.vert" "shaders/*.geom" "shaders/*.frag" "shaders/*.comp")

  message("Source files")
  foreach(line IN LISTS source_files)
    message("${line}")
  endforeach()

  message("Header files")
  foreach(line IN LISTS header_files)
    message("${line}")
  endforeach()

  message("Shader files")
  foreach(line IN LISTS shader_files)
    message("${line}")
  endforeach()

  add_definitions(-g -Wall -Wextra -Wno-comment -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable -DCORRECTION)

  add_executable(${dir} ${source_files} ${header_files} ${shader_files})

  target_include_directories(${dir} 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/stb/include
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/tinyobjloader/include
  )

  target_compile_features(${dir} PUBLIC cxx_std_17)

  target_link_libraries(${dir}
    PUBLIC
        glfw
        imgui
        imgui_backend
        tools
        glm::glm
  )
endforeach()

target_include_directories(programme_1 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/glad/gl33/include)
# target_include_directories(programme_2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/glad/gl43/include)