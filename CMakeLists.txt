# Specify the minimum version of CMake that is required to process this file.
# This command is used to ensure that a certain set of CMake features is available to the build.
cmake_minimum_required(VERSION 3.15)

# Declare the name of the project and set it as 'PROG2002'. This command is used to set the project name
# and optionally its version, description, and other attributes.
project(PROG2002)

# Set the C++ standard to C++17 for all targets in the project. This variable denotes
# the desired C++ standard version (e.g., 11, 14, 17, 20).
set(CMAKE_CXX_STANDARD 17)

# Enforce that the specified C++ standard is mandatory. If set to ON, then it's a
# requirement to have C++17. If the compiler does not support it, CMake will fail.
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Disable compiler-specific extensions to the C++ standard. This ensures that the
# codebase remains portable and can be compiled using any compliant C++ compiler.
set(CMAKE_CXX_EXTENSIONS OFF)

# Locate the OpenGL package on the system. This is essential for projects that
# need to link against OpenGL. The REQUIRED argument stops the configuration process
# with an error message if OpenGL is not found.
find_package(OpenGL REQUIRED)

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

# Add external dependencies or third-party libraries for the project. The 'add_subdirectory'
# command adds a sub-directory to the build, which should contain its own 'CMakeLists.txt'.
# 'glad', 'glfw', 'glm', and 'tinyobjloader' are some common libraries used in OpenGL projects.
# In this project, these libraries are git submodules; make sure you pull them correctly
# (e.g., git clone ..... --recursive)
add_subdirectory(external/glad)
add_subdirectory(external/glfw)
add_subdirectory(external/glm)
add_subdirectory(external/tinyobjloader)

# Create a header-only interface for the 'stb' library. Since 'stb' typically consists
# of single-header libraries, this approach makes it easy to integrate into the project.
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE external/stb)

# Add subdirectories for OpenGL lab examples. These likely contain individual
# projects or exercises that students might work on.
add_subdirectory(examples/example_1)
add_subdirectory(examples/example_2)
add_subdirectory(examples/example_3)
add_subdirectory(examples/example_4)
add_subdirectory(examples/example_5)

# Add a subdirectory for a labs. This line is commented out, possibly because the
# labs is either under development or optional.
# add_subdirectory(labs)

# Add a subdirectory for a framework. This line is commented out, possibly because the
# framework is either under development or optional.
# add_subdirectory(framework)

# Add a subdirectory for assignments. Like the framework, this is commented out,
# potentially to be enabled later when assignments are ready.
# add_subdirectory(assignment)
