/* =======================================================================================
    Copyright 2023 NTNU

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
=======================================================================================*/

#include "shaders/triangle.h"
#include "shaders/square.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <set>
#include <cmath>

// -----------------------------------------------------------------------------
// FUNCTION PROTOTYPES
// -----------------------------------------------------------------------------
GLuint CompileShader(const std::string& vertexShader,
                     const std::string& fragmentShader);
GLuint CreateTriangle();
GLuint CreateSquare();

void CleanVAO(GLuint &vao);

void GLFWErrorCallback(int code, const char* description);
void GLAPIENTRY MessageCallback(GLenum source,
                                GLenum type,
                                GLuint id,
                                GLenum severity,
                                GLsizei length,
                                const GLchar* message,
                                const void* userParam);

// -----------------------------------------------------------------------------
// ENTRY POINT
// -----------------------------------------------------------------------------
int main()
{
  // Initialization of GLFW
  glfwSetErrorCallback(GLFWErrorCallback);  // Setting an error callback for GLFW to capture any issues.
  if(!glfwInit())
    {
    std::cerr << "GLFW initialization failed." << '\n';
    std::cin.get();

    return EXIT_FAILURE;
    }

  // Setting window hints to define the desired OpenGL context properties before creating the OpenGL context
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  auto window = glfwCreateWindow(1200, 1200, "Lab02", nullptr, nullptr);
  if (window == nullptr)
    {
    std::cerr << "GLFW failed on window creation." << '\n';
    std::cin.get();

    glfwTerminate();

    return EXIT_FAILURE;
    }

  // Setting the OpenGL context.
  glfwMakeContextCurrent(window);

  // Initialize GLAD before any OpenGL function is called.
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
    std::cout << "Failed to initialize GLAD" << std::endl;
    glfwTerminate();
    return EXIT_FAILURE;
    }

  // Enabling capture of debug output to get messages about potential issues.
  glEnable(GL_DEBUG_OUTPUT);
  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
  glDebugMessageCallback(MessageCallback, 0);
  glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

  // Print OpenGL context information.
  std::cout << "Vendor: " << glGetString(GL_VENDOR) << "\n";
  std::cout << "Renderer: " << glGetString(GL_RENDERER) << "\n";
  std::cout << "OpenGL version: " << glGetString(GL_VERSION) << "\n";

  // Creating a square and its associated shader program.
  auto squareVAO = CreateSquare();
  auto squareShaderProgram = CompileShader(squareVertexShaderSrc,
                                           squareFragmentShaderSrc);

  // Creating a triangle and its associated shader program.
  auto triangleVAO = CreateTriangle();
  auto triangleShaderProgram = CompileShader(triangleVertexShaderSrc,
                                             triangleFragmentShaderSrc);

  glClearColor(1.0f, 0.0f, 0.0f, 1.0f);

  bool alternate = false;
  double currentTime = 0.0;
  double lastTime = 0.0;
  glfwSetTime(0.0);
  while(!glfwWindowShouldClose(window))
    {
    glfwPollEvents();

    // Time management for alternate flag
    currentTime = glfwGetTime();
    if (currentTime - lastTime > 1.0)
      {
      alternate = !alternate;
      lastTime = currentTime;
      }

    glClear(GL_COLOR_BUFFER_BIT);

    auto greenValue = (sin(currentTime) / 2.0f) + 0.5f;

    // -----------------------------------------------------------------------------
    // Draw SQUARE with dynamic green value
    // -----------------------------------------------------------------------------
    // 1. Every shader and program object has a global dictionary of all its uniform variables.
    //    This step fetches the location of the uniform variable "u_Color" within the squareShaderProgram.
    auto vertexColorLocation = glGetUniformLocation(squareShaderProgram, "u_Color");

    // 2. Bind (or activate) the shader program. Once a shader program is active, we can update its uniforms.
    glUseProgram(squareShaderProgram);

    // 3. Bind the VAO. This ensures we're using the right data (vertices, indices) for our draw call.
    glBindVertexArray(squareVAO);

    // 4. Update the "u_Color" uniform value within the currently active shader program.
    //    We use the glUniform4f function because the uniform is a vec4 (4 floats).
    //    The green component is dynamically calculated based on the sine function to make it pulsate over time.
    glUniform4f(vertexColorLocation, 0.0f, greenValue, 0.0f, 1.0f);

    // 5. Issue a draw call to OpenGL. This will use the bound VAO and the active shader program.
    //    glDrawElements indicates we're using indexed drawing,
    //    and we specify GL_TRIANGLES to denote the drawing mode.
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (const void*)0);

    // -----------------------------------------------------------------------------
    // Draw TRIANGLE with alternate flag
    // -----------------------------------------------------------------------------
    // 1. Fetch the location of the uniform variable "u_AlternateFlag" within the triangleShaderProgram.
    auto alternateFlagLocation = glGetUniformLocation(triangleShaderProgram, "u_AlternateFlag");

    // 2. Bind (or activate) the shader program for the triangle.
    glUseProgram(triangleShaderProgram);

    // 3. Bind the VAO for the triangle.
    glBindVertexArray(triangleVAO);

    // 4. Update the "u_AlternateFlag" uniform within the currently active shader program.
    //    We use glUniform1ui because the uniform is an unsigned integer.
    //    The value we pass in depends on the "alternate" flag.
    glUniform1ui(alternateFlagLocation, static_cast<unsigned int>(alternate));

    // 5. Issue a draw call to OpenGL for the triangle.
    //    glDrawArrays is used here because the triangle isn't using indexed drawing.
    //    We specify GL_TRIANGLES to denote the drawing mode.
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glfwSwapBuffers(window);

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
      {
      break;
      }
    }

  // Cleanup
  glUseProgram(0);
  glDeleteProgram(triangleShaderProgram);
  glDeleteProgram(squareShaderProgram);

  CleanVAO(triangleVAO);
  CleanVAO(squareVAO);

  glfwTerminate();

  return EXIT_SUCCESS;
}

// -----------------------------------------------------------------------------
// COMPILE SHADER
// -----------------------------------------------------------------------------
GLuint CompileShader(const std::string& vertexShaderSrc,
                     const std::string& fragmentShaderSrc)
{
  // Convert shader source code from std::string to raw char pointer.
  auto vertexSrc = vertexShaderSrc.c_str();
  auto fragmentSrc = fragmentShaderSrc.c_str();

  // Compile vertex shader
  auto vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexSrc, nullptr);
  glCompileShader(vertexShader);

  // Compile fragment shader
  auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentSrc, nullptr);
  glCompileShader(fragmentShader);

  // Link shaders into a program
  auto shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);

  // Clean up shader objects as they're no longer needed after linking
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  return shaderProgram;
}

// -----------------------------------------------------------------------------
//  CREATE TRIANGLE
// -----------------------------------------------------------------------------
GLuint CreateTriangle()
{
  // Triangle vertices with interleaved position and color data
  GLfloat triangle[3*3*2] =
    {
      -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
      0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f
    };

  GLuint triangleVBO; // Vertex Buffer Object
  GLuint triangleVAO; // Vertex Array Object

  // Creating a Vertex Array Object (VAO). In OpenGL 4.5 and newer, glCreateVertexArrays can be used as an alternative for improved safety.
  glGenVertexArrays(1, &triangleVAO);
  glBindVertexArray(triangleVAO);

  // Generate and bind VBO
  glGenBuffers(1, &triangleVBO);
  glBindBuffer(GL_ARRAY_BUFFER, triangleVBO);

  glBufferData(GL_ARRAY_BUFFER, sizeof(triangle), triangle, GL_STATIC_DRAW);

  // Vertex positions
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  // Vertex colors
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  // Clean up
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  glDeleteBuffers(1, &triangleVBO);

  return triangleVAO;
}

// -----------------------------------------------------------------------------
//  CREATE SQUARE
// -----------------------------------------------------------------------------
GLuint CreateSquare()
{
  // Square vertices with interleaved position and color data
  GLfloat squareVertices[4*3*2] =
    {
      -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
      0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
      0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
      -0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 0.0f
    };

  GLuint squareIndices[2*3] =
    {
      0, 1, 2,
      2, 3, 0
    };

  GLuint squareVBO; // Vertex Buffer Object
  GLuint squareEBO; // Element Buffer Object
  GLuint squareVAO; // Vertex Array Object

  // Creating a Vertex Array Object (VAO). In OpenGL 4.5 and newer, glCreateVertexArrays can be used as an alternative for improved safety.
  glGenVertexArrays(1, &squareVAO);
  glBindVertexArray(squareVAO);

  // Generate and bind VBO
  glGenBuffers(1, &squareVBO);
  glBindBuffer(GL_ARRAY_BUFFER, squareVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(squareVertices), squareVertices, GL_STATIC_DRAW);

  // Generate and bind EBO
  glGenBuffers(1, &squareEBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, squareEBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(squareIndices), squareIndices, GL_STATIC_DRAW);

  // Vertex positions
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  // Vertex colors
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  // Clean up
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  glDeleteBuffers(1, &squareVBO);

  return squareVAO;
}

// -----------------------------------------------------------------------------
// CLEAN VAO
// -----------------------------------------------------------------------------
void CleanVAO(GLuint &vao)
{
  glBindVertexArray(vao);
  GLint maxVertexAttribs;
  glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxVertexAttribs);
  for(GLint i = 0; i < maxVertexAttribs; i++)
    {
    glDisableVertexAttribArray(i);
    }
  glBindVertexArray(0);
  glDeleteVertexArrays(1, &vao);
  vao = 0;
}

// -----------------------------------------------------------------------------
//  GLFW ERROR CALLBACK
// -----------------------------------------------------------------------------
void GLFWErrorCallback(int code, const char* description)
{
  std::cerr << "GLFW Error " << code << ": " << description << "\n";
}

// -----------------------------------------------------------------------------
//  MESSAGE CALLBACK
// -----------------------------------------------------------------------------
void GLAPIENTRY MessageCallback(GLenum source,
                                GLenum type,
                                GLuint id,
                                GLenum severity,
                                GLsizei length,
                                const GLchar* message,
                                const void* userParam)
{
  // Outputs debugging messages from OpenGL
  std::cerr << "GL CALLBACK: "
    << (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "")
    << " type = 0x" << type
    << ", severity = 0x" << severity
    << ", message = " << message << "\n";
}
