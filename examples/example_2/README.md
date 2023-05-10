# Example 2: Creating a Simple OpenGL Application (II)
## Introduction
In *Example 1*, we learned how to set up a basic OpenGL project that could render a triangle on a scene. In this Example, we will build upon the concepts learned in *Example 1* and expand our knowledge by:

- Setting up a project using *CMake* instead of manual configuration (libraries, paths, etc.).
- Adding color data to our triangle using /Vertex Attributes/.
- Generating a square using /Indexed Drawing/.
- Dynamically changing the color of the square using /Uniforms/.

## Tasks
### Task 0: Initial Setup

- Visual Studio 19 supports CMake by default.

- (Alternative) Install CMake. Windows users can visit cmake.org and download/install the latest version (3.18). Linux users can use the package manager to install cmake:

```bash
apt install cmake cmake-curses-gui cmake-qt-gui
```

- Generate a simple *CMakeLists.txt* file to initialize the project and add an executable file (*Example02*) from the source file (*main.cpp*) along with the required external libraries (*glfw* and *glew*).

- Create the *main.cpp* file and develop a simple application (this can be an empty application or one that prints something on the screen).

- Use *cmake* (or *cmake-gui*) to configure the project for your platform (e.g., Visual Studio or Unix Makefiles for Linux). Tip: Have the /cmake configuration process/ output the results to a *\build\* folder within your source project (e.g., ...\example_1\build).

- Compile and run your program.

### Task 1: Create Your First Window and OpenGL Context with GLFW

- Modify your *CMakeLists.txt* file to locate *GLFW* and link your application with *GLFW*.

- Add the necessary code to your *main.cpp* file to create a *GLFW* window.

  + Keep the window open until the user presses the escape key.
  + Inform the user about any possible errors that may occur during the initialization of GLFW or the creation of the window.
  + Properly terminate the application when the user closes the window.
  + Set the OpenGL context as the current context.

### Task 2: Initialize GLEW and Clear the Screen

- Modify your *CMakeLists.txt* file to locate *GLEW* and *OpenGL*, and link your application with *GLEW* and *OpenGL*.

- Initialize *GLEW*.

- Use basic *OpenGL* functions to set the background color of the window to a color of your choice.

### Task 3: Create a Triangle and Display It on the Screen

- Display a triangle centered at the origin using /VAO/, /VBO/, and /Vertex Attributes/ (without using /EOB/).
  +  Each vertex should have /position/ and /color/ attributes.
  + Create a simple vertex shader that passes the vertex coordinates unchanged and assigns a color to each vertex.
  + Create a simple /Fragment shader/ that sets the triangle's color based on the color received from the /Vertex Shader/.

### Task 4: Create a Square That Changes Color Over Time

- Display a square centered in origin. Use /VAO/, /VBO/, /EBO/ and /Vertex Attributes/
  + Each vertex should have only the /position/ attribute.
  + Create a simple vertex shader that forwards the vertex coordinates unchanged.
  + Create a simple /Fragment shader/ that sets the triangle to the color received by an uniform.
  + Make the color of the triangle change periodically over time from /black/ to /green/.
  + Ensure that the square is drawn before the triangle to make both visible in the scene.
