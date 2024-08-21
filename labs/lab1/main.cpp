#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <cstdlib>
#include <ios>
#include <iostream>

void error_callback(int code, const char* description);
void GLAPIENTRY message_callback(GLenum source, GLenum type, GLuint id,
                                 GLenum severity, GLsizei length,
                                 const GLchar* message, const void* userParam);

// TODO
class Context {
 public:
};

int main() {
  glfwSetErrorCallback(error_callback);

  // Defer: glfwTerminate()
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW." << std::endl;
    return EXIT_FAILURE;
  }

  glfwWindowHint(GLFW_RESIZABLE, false);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  auto window = glfwCreateWindow(800, 600, "Hello Triangle", nullptr, nullptr);
  if (window == nullptr) {
    std::cerr << "Failed to create GLFW window." << std::endl;
    glfwTerminate();
    return EXIT_FAILURE;
  }

  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << "Failed to initialize GLAD." << std::endl;
    glfwTerminate();
    return EXIT_FAILURE;
  }

  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(message_callback, 0);

  std::cout << "Vendor: " << glGetString(GL_VENDOR) << "\n";
  std::cout << "Renderer: " << glGetString(GL_RENDERER) << "\n";
  std::cout << "OpenGL version: " << glGetString(GL_VERSION) << "\n";

  glClearColor(0.5f, 0.0f, 0.0f, 1.0f);
  GLfloat triangle[3 * 2] = {-0.5f, -0.5f, 0.5f, -0.5f, 0.0f, 0.5f};

  GLuint vertexArrayId;
  glGenVertexArrays(1, &vertexArrayId);
  glBindVertexArray(vertexArrayId);

  GLuint vertexBufferId;
  glGenBuffers(1, &vertexBufferId);
  glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
  glBufferData(GL_ARRAY_BUFFER, sizeof(triangle), triangle, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, nullptr);
  glEnableVertexAttribArray(0);

  // TODO: Move shader code to separate files.
  // TODO: Shader hot reload?
  const std::string vertexShaderSrc = R"(
#version 430 core
layout(location = 0) in vec4 position;
void main()
{
    gl_Position = position;
}
)";

  const std::string fragmentShaderSrc = R"(
#version 430 core
out vec4 color;
void main()
{
    color = vec4(1);
}
)";

  auto vertexShader = glCreateShader(GL_VERTEX_SHADER);
  const GLchar* vss = vertexShaderSrc.c_str();
  glShaderSource(vertexShader, 1, &vss, nullptr);
  glCompileShader(vertexShader);

  auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  const GLchar* fss = fragmentShaderSrc.c_str();
  glShaderSource(fragmentShader, 1, &fss, nullptr);
  glCompileShader(fragmentShader);

  auto shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
  glUseProgram(shaderProgram);

  /* === MAIN RENDER LOOP ===
   * Until the window is closed or the ESC key is pressed, we:
   * - Poll for events (like input).
   * - Clear the screen.
   * - Draw our triangle.
   * - Swap buffers to show our triangle.
   */
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glfwSwapBuffers(window);
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) break;
  }

  /* === CLEANUP ===
   * Freeing resources after they are no longer needed is good practice.
   * We delete our buffers and terminate GLFW.
   */
  glDisableVertexAttribArray(0);
  glDeleteBuffers(1, &vertexBufferId);
  glDeleteVertexArrays(1, &vertexArrayId);
  glDeleteProgram(shaderProgram);

  glfwTerminate();

  return EXIT_SUCCESS;
}

void error_callback(int code, const char* description) {
  std::cerr << "GLFW Error (0x" << std::hex << code << "): " << description
            << "\n";
}

void GLAPIENTRY message_callback(GLenum source, GLenum type, GLuint id,
                                 GLenum severity, GLsizei length,
                                 const GLchar* message, const void* userParam) {
  std::cerr << "OpenGL Callback: "
            << (type == GL_DEBUG_TYPE_ERROR ? "** ERROR **" : "") << " Type: 0x"
            << type << ", Severity: 0x" << severity << ", Message: " << message
            << "\n";
}
