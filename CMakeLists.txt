cmake_minimum_required(VERSION 3.15)
project(SnakeOnACube CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

include_directories(src ${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS})

set(SOURCES
    src/main.cpp
    src/cube_topology.cpp
    src/game_state.cpp
    src/camera.cpp
    src/renderer.cpp
)

add_executable(snake_cube ${SOURCES})
target_link_libraries(snake_cube PRIVATE ${GLUT_LIBRARIES} ${OPENGL_LIBRARIES} glu32)

# Headless Unit Tests
add_executable(test_transitions tests/test_transitions.cpp src/cube_topology.cpp)
target_include_directories(test_transitions PRIVATE src)
