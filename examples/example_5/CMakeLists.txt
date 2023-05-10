cmake_minimum_required(VERSION 3.12)

project(example_5)

find_package(OpenGL REQUIRED)

add_executable(example_5
	main.cpp
	shaders/shader.h)


set(MODELS_PATH "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources/models")
string(REPLACE "/" "\/" ESCAPED_MODELS_PATH ${MODELS_PATH})
target_compile_definitions(${PROJECT_NAME}
  PRIVATE
  MODELS_DIR="${ESCAPED_MODELS_PATH}"
  TINYOBJLOADER_IMPLEMENTATION)

target_link_libraries(example_5
	glfw
	glm
  glad
  tinyobjloader
	OpenGL::GL)

add_custom_command(
  TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy
  ${CMAKE_CURRENT_SOURCE_DIR}/resources/models/teacup.obj
  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources/models/teacup.obj
  )

add_custom_command(
  TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy
  ${CMAKE_CURRENT_SOURCE_DIR}/resources/models/teacup.mtl
  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources/models/teacup.mtl
  )
