cmake_minimum_required(VERSION 3.15)

project(PROG2002)

# Set the C++ standard to C++17 for all targets in the project. This variable denotes
# the desired C++ standard version (e.g., 11, 14, 17, 20).
set(CMAKE_CXX_STANDARD 23)

# Enforce that the specified C++ standard is mandatory. If set to ON, then it's a
# requirement to have C++17. If the compiler does not support it, CMake will fail.
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Disable compiler-specific extensions to the C++ standard. This ensures that the
# codebase remains portable and can be compiled using any compliant C++ compiler.
set(CMAKE_CXX_EXTENSIONS OFF)

# Locate the required packages for the project
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(glew REQUIRED)

# Define the output directories for the built archives, libraries, and runtime
# executables respectively. These settings help in organizing the built files.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# GLFW has a CMake script for us to use, but it has some unnecessary settings that are on
# by default. We just disable these and then include their CMake script, and link our
# executable to their CMake library target 'glfw'. Thanks to Nils P. Skålerud.
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
set(GLFW_VULKAN_STATIC OFF CACHE BOOL "" FORCE)

add_subdirectory(labs/lab1)

