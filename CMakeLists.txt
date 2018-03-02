cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_STANDARD 14)

find_package(OpenGL REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/include)

option(GLFW_BUILD_DOCS OFF)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
add_subdirectory(glfw)

add_executable(Test src/Test.cpp src/glad.c)
target_link_libraries(Test glfw ${OPENGL_gl_LIBRARY})

add_executable(Water1 src/Water1.cpp src/Skybox.cpp src/Waves.cpp src/glad.c)
target_link_libraries(Water1 glfw ${OPENGL_gl_LIBRARY})