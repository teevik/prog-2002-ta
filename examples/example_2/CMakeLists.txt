# Minimum required CMake version.
cmake_minimum_required(VERSION 3.15)

# Define the project name.
project(example_2)

# Locate the OpenGL package on the system.
find_package(OpenGL REQUIRED)

# Define the 'example_2' executable and its associated source files.
add_executable(example_2
	main.cpp
	shaders/triangle.h
	shaders/square.h)

# Link the specified libraries against the 'example_2' executable.
target_link_libraries(example_2
  PRIVATE
  glad
  glfw
  OpenGL::GL)
