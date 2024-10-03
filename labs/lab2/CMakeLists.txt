project(lab2)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} OpenGL::GL glfw GLEW::glew glm::glm framework)

# Copy the assets directory to the build directory
add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets)
