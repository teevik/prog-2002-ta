# Set the minimum required version of CMake for this project
cmake_minimum_required(VERSION 3.15)

# Define the project name
project(example_4)

# Look for the OpenGL package, which is required by this project
find_package(OpenGL REQUIRED)

# Add an executable target with source files and headers
add_executable(${PROJECT_NAME}
  main.cpp
  shaders/square.h)

# Specify the include directories for the target
target_include_directories(${PROJECT_NAME}
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Define a preprocessor macro for the STB image library
target_compile_definitions(${PROJECT_NAME}
  PRIVATE
  STB_IMAGE_IMPLEMENTATION)

# Set up a compile definition for the texture directory
# Escaping the path ensures it's recognized correctly by the preprocessor
# This makes it easier to use absolute paths in the source code for texture loading
set(TEXTURES_PATH "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources/textures")
string(REPLACE "/" "\/" ESCAPED_TEXTURES_PATH ${TEXTURES_PATH})
target_compile_definitions(${PROJECT_NAME} PRIVATE TEXTURES_DIR="${ESCAPED_TEXTURES_PATH}")

# Link the target with necessary libraries
target_link_libraries(${PROJECT_NAME}
  PRIVATE
  glfw
  glm
  glad
  OpenGL::GL
  stb)

# Custom command to copy the 'cat.png' texture to the expected build directory after the build
# This ensures that the texture is available at runtime, regardless of where the executable is invoked from
add_custom_command(
  TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy
  ${CMAKE_CURRENT_SOURCE_DIR}/resources/textures/cat.png
  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources/textures/cat.png)

# Custom command to copy the 'dog.png' texture in a similar manner to 'cat.png'
add_custom_command(
  TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy
  ${CMAKE_CURRENT_SOURCE_DIR}/resources/textures/dog.png
  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources/textures/dog.png)
