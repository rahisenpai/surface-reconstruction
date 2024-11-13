cmake_minimum_required(VERSION 3.10)

# Set the project name
project(SurfaceReconstruction)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include directories
include_directories(include)

# Find the required packages
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

# Add the executable
file(GLOB SOURCES "src/*.cpp")
add_executable(SurfaceReconstruction ${SOURCES})

# Link the libraries
target_link_libraries(SurfaceReconstruction OpenGL::GL GLEW::GLEW glfw GLU glm::glm)