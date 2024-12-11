project(framework)

add_library(${PROJECT_NAME} 
  shader.cpp
  buffer.cpp
  pipeline.cpp
  window.cpp
  texture.cpp
)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME} OpenGL::GL glfw GLEW::glew glm::glm)

target_compile_definitions(${PROJECT_NAME} PRIVATE STB_IMAGE_IMPLEMENTATION)
