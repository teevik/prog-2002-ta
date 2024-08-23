project(lab1)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} glm glfw glad OpenGL::GL)

add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets)

#  path to resource folder @formatter:off
target_compile_definitions(${PROJECT_NAME} PRIVATE ASSETS_DIR="${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets/")
