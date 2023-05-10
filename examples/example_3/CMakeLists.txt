# Set the minimum required CMake version to 3.15
cmake_minimum_required(VERSION 3.15)

# Define the project as "example_3"
project(example_3)

# Find the required OpenGL package
find_package(OpenGL REQUIRED)

# Create an executable named "example_3" from source files
add_executable(example_3
	main.cpp
	shaders/square.h)

# Link libraries to the "example_3" executable
target_link_libraries(example_3
	PRIVATE
	glad
	glfw
	glm
	OpenGL::GL)
