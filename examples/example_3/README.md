# Example 3: Working with Matrices (III)

 **NOTE: The code for this example is complete and serves to illustrate the theory covered in the course with examples. Use the following instructions to clarify aspects or challenge yourself by starting the example from scratch.**

## Introduction
In *Example 1*, we learned how to set up a basic OpenGL project to render a triangle in a scene.
In *Example 2*, we worked on setting up a project using CMake and explored working with colors, uniforms, and a variable sent to the shader to change the colors of our objects.
In this example, we will learn how to use uniforms to access the vertex positions of our objects and apply various transformations during runtime.

- We will explore the matrix calculation functions provided by the glm extension, which will be included in our project using CMake.
- We will directly pass data to our shader during runtime and learn how to modify the positions of our vertices without needing to work on the Vertex Buffer Object (VBO).
- We will learn how to use matrices to have more control over the camera and the viewport.

## TASKS

### Task 0: Initial setup

- Follow the same steps as in *Example 2* to set up a cmake project.
- Additionally, include the glm library by downloading it from their website and adding the folder to your project's external folder.
- Update your CMakeLists.txt file to include the line *find_package(glm REQUIRED)* along with the equivalent command for OpenGL.
- Apart from that, the setup will be the same as in Example 2.

### Task 1: Create a window and set up GLEW and GLFW

- Write the necessary code to set up GLEW and GLFW as usual. You should have a window and clear the screen each frame.
- This step should be the same as in Example 2.
- Make sure to include *glm/glm.hpp* in the list of includes at the top of *main.cpp*.

### Task 2: Create a square and display it on the screen

- Display a square at the center of the screen using Vertex Array Object (VAO), Vertex Buffer Object (VBO), and optionally Element Buffer Object (EBO) along with Vertex Attributes.
  + Each vertex can have either just a position attribute or both position and color attributes. The choice doesn't matter for this example.
  + Create a simple vertex shader that passes the vertex coordinates as they are.
  + Create a simple fragment shader that assigns a color to the square using any method you prefer.
  + Draw the square on the screen.

### Task 3: Create Matrices for the Camera

- Create a uniform variable in your vertex shader to handle the view matrix.

  + The variable should be of type `mat4`. You can give it a default value, such as `mat4(1)`, to avoid issues if it's improperly initialized.
  + Multiply the uniform variable with the position attribute.

- Create another uniform variable in your vertex shader to handle the projection matrix.

  + For now, initialize it in the same way as the view matrix.
  + Multiply the projection matrix with the view matrix.
  + The final multiplication order should be `projection * view * position`.

- Create a function that takes an unsigned integer (you can use `GLuint`) as a parameter. This function will initialize our camera.

  + Inside the function, use glm to create a `mat4` matrix for our view matrix. To use the necessary functions, include `glm/gtc/matrix_transform` at the beginning of your file.
  + Initialize the view matrix using the `glm::lookat` command. The first parameter should be the position of the camera (e.g., `glm::vec3(0,0,1)`), the second parameter should be the point the camera is looking at (e.g., `glm::vec3(0,0,0`) to look at the origin), and the third parameter should be a vector representing the "up" axis (e.g., `glm::vec3(0,1,0)`).
  + Now, create another `mat4` matrix for our projection matrix.
  + Initialize it using `glm::ortho` to create an orthographic projection matrix. Since this is a 2D project, we won't need a perspective matrix.
  + The values represent the aspect ratio of your window. If the window is square, the values should be (-1,1,-1,1). If the aspect ratio is different, the longer side should be 1, and the shorter side should have values proportional to its size compared to the longer side. For example, if the window is twice as wide as it is tall, the values should be (-1,1,-0.5,0.5). Feel free to experiment with these values to see how they affect your output.

- Within the function, find and assign the values of these matrices to the uniforms you created in your shaders.

  + To do this, first find the location of the uniform, either by specifying it in the shader using `(location * X)` where `X` represents a location not previously used for uniforms, or use the `glGetUniformLocation` function. The first input will be the unsigned integer parameter of the function you are working on, and the second should be the name of the uniform in the shader.
  + Use the obtained data location as the first parameter for the `glUniformMatrix4fv` function. Provide a value specifying the number of matrices you are modifying (1), a boolean to indicate whether you want to transpose the matrix (false), and use `glm::value_ptr along` with your matrix as the argument. To use `value_ptr`, include `glm/gtc/type_ptr.hpp` at the beginning of your file.
- Call the function from inside your main loop, after calling `glUseProgram`, using your shader program as the input.

### Task 4: Create Matrices for Object Transformations

- Create another uniform variable in your vertex shader to hold the transformation matrix. This variable should also be of type `mat4`.

  + Multiply the new transformation matrix with the other two uniform matrices. The resulting multiplication should look like `projection * view * transformation * position`.

- Create another function that takes a float (representing the time passed since the start of the application) and an unsigned integer as parameters.

  + Inside the function, create a matrix for translating your objects. This matrix should be of type `mat4`.

  + Initialize it using `glm::translate`, which takes `mat4(1)` as the first parameter, and a vector indicating how much you want to change the object's position along the x, y, and z axes (e.g., `glm::vec3(0,0,5)`). You can also use the first parameter of the function to make the object move based on time, for example, using a sinus curve (`glm::sin`).

-Create another matrix for rotating your objects. This matrix should also be of type mat4.

  + Initialize it using `glm::rotate`, which takes `mat4(1)` as the first parameter, the number of degrees in radians by which you want to rotate the object, and the axis around which you want to rotate it (as a `vec3`). You can also use the first parameter of your function to make the object rotate over time.

- Create a third matrix for scaling your objects. This matrix should also be of type `mat4`.

  + Initialize it using glm::scale, which takes mat4(1) as the first parameter, and a vector indicating how much to scale the object along each axis. You can also use the first parameter of your function to make the object scale over time.

- Create a fourth matrix to combine the previous matrices into a transformation matrix. This matrix should also be of type mat4.

  + Initialize it by multiplying the matrices you just created in the following order: (translation * rotation * scale). It's important to multiply them in this order for the result to be correct.

- Find the location of the uniform for your transformation matrix, just as you did for the view and projection matrices in the previous step. You can use the prewritten location or the shader program to obtain the location.

- Pass the transformation matrix to the uniform using the same method as before, using the `glUniformMatrix4fv` function.

- Call your function inside the main loop after calling `glUseProgram`, with the shader program as the second input.

  + The first input should be the time that has passed since the program started. You can obtain this by calling `glfwSetTime(0.0)` before the main loop and then `glfwGetTime()` inside the loop to get the current time value.

