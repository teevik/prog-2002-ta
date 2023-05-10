# Example 4: Textures

## Introduction

In Example 3, you learned how to use matrix transformations with OpenGL shaders and the math library. This week, we will focus on textures.

- We will use the stb image library to load images that we can use as textures.
- We will write OpenGL shaders to render the texture images.
- Steps

### Step 0: Initial setup

- Just like in previous examples, set up a CMake project as usual. Ensure that you have all the required libraries (stb, glm, glad, and glfw) and the CMakeLists.txt file by downloading the example 4 folder.

### Step 1: Create a window and set up GLAD and GLFW

- As always, write the necessary code to set up GLAD and GLFW in order to create and clear your drawing window.
- Include the required libraries for this example, such as `glm/glm.hpp` and `stb_image.h`, and any other necessary dependencies.

### Step 2: Load the image you want to use as a texture

- To use an image as a texture, we need to read it from a file. We can use a library called stb_image.h, which is a small and cross-platform image loader. It can read various image formats like BMP, JPEG, and PNG into arrays of raw pixels. You can find stb_image.h here: https://github.com/nothings/stb. The function we'll be using is called stbi_load, but you can explore alternative options by researching or reading the documentation.

### Step 3: Create and initialize the OpenGL texture

Similar to Vertex Buffer Objects (VBOs) and Vertex Array Objects (VAOs), textures in OpenGL need to be generated first. We use functions like glGenTextures or glCreateTextures to create these objects. Textures also require an additional attribute called texture coordinates, often referred to as uv-coordinates. These coordinates are added to a vertex, which typically has a layout like this: [X, Y, R, G, B, U, V].

- Create vertices with texture coordinates.
- Create and bind an OpenGL texture object.
- Load the image data onto the created GL texture object.
- Set the necessary texture parameters.

### Step 4: Render the texture

To render the texture, we need to modify the vertex and fragment shaders to include the texture. We specify texture slots using uniforms.

- Set up and enable all the required vertex attributes (position, color, and texture coordinates) for rendering the texture.
- Draw the rectangle using the appropriate VAO, EBO, and UniformMatrix data.
