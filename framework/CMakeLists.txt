project(framework)

add_library(${PROJECT_NAME} 
  shader.cpp
  buffer.cpp
)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME} OpenGL::GL glfw GLEW::glew glm::glm)
