#include <cassert>
#include <filesystem>
#include <optional>
#include <vector>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <cstdlib>
#include <fstream>
#include <glm/glm.hpp>
#include <ios>
#include <iostream>
#include <span>
#include <string>

using std::filesystem::path;

void error_callback(int code, const char* description);
void GLAPIENTRY message_callback(GLenum source, GLenum type, GLuint id,
                                 GLenum severity, GLsizei length,
                                 const GLchar* message, const void* userParam);

enum class BufferType {
  Vertex = GL_ARRAY_BUFFER,
  Index = GL_ELEMENT_ARRAY_BUFFER,
};

enum class BufferUsage {
  Static = GL_STATIC_DRAW,
  Dynamic = GL_DYNAMIC_DRAW,
  Stream = GL_STREAM_DRAW,
};

class Buffer {
 private:
  uint32_t id = 0;
  BufferType type;

 public:
  template <typename T>
  Buffer(BufferType type, BufferUsage usage, std::span<T> data) : type(type) {
    glGenBuffers(1, &id);

    glBindBuffer(static_cast<GLenum>(type), id);
    glBufferData(static_cast<GLenum>(type), sizeof(T) * data.size(),
                 data.data(), static_cast<GLenum>(usage));
  };

  // Move constructor
  Buffer(Buffer&& object) noexcept : id(object.id), type(object.type) {
    object.id = 0;
  }

  ~Buffer() {
    if (id) glDeleteBuffers(1, &id);
  }

  void bind() const { glBindBuffer(static_cast<GLenum>(type), id); }
};

std::string read_file_to_string(const std::filesystem::path path) {
  std::ifstream file(path);
  std::string content((std::istreambuf_iterator<char>(file)),
                      (std::istreambuf_iterator<char>()));

  return content;
}

enum class ShaderType {
  Vertex = GL_VERTEX_SHADER,
  Fragment = GL_FRAGMENT_SHADER,
};

std::optional<uint32_t> compile_shader(const std::string& source,
                                       ShaderType shaderType) {
  auto shader_id = glCreateShader(static_cast<GLenum>(shaderType));

  const char* raw_source = source.c_str();
  glShaderSource(shader_id, 1, &raw_source, nullptr);
  glCompileShader(shader_id);

  int32_t shader_did_compile;
  glGetShaderiv(shader_id, GL_COMPILE_STATUS, &shader_did_compile);

  if (!shader_did_compile) {
    int32_t errorLength;
    glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &errorLength);

    auto errorMessage = std::make_unique<char[]>(errorLength);
    glGetShaderInfoLog(shader_id, errorLength, &errorLength,
                       errorMessage.get());

    std::cerr << "Failed to compile shader!" << std::endl;
    std::cerr << errorMessage.get() << std::endl;

    glDeleteShader(shader_id);

    return std::nullopt;
  }

  return shader_id;
};

// TODO: Shader hot reload?
class Shader {
 private:
  uint32_t id = 0;

 public:
  Shader(std::filesystem::path vertex_shader_file,
         std::filesystem::path fragment_shader_file) {
    id = glCreateProgram();

    auto vertex_shader_source = read_file_to_string(vertex_shader_file);
    auto fragment_shader_source = read_file_to_string(fragment_shader_file);

    auto vertex_shader =
        compile_shader(vertex_shader_source, ShaderType::Vertex).value();
    auto fragment_shader =
        compile_shader(fragment_shader_source, ShaderType::Fragment).value();

    glAttachShader(id, vertex_shader);
    glAttachShader(id, fragment_shader);

    glLinkProgram(id);
    glValidateProgram(id);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
  };

  Shader(Shader&& shader) noexcept : id(shader.id) { shader.id = 0; }

  ~Shader() {
    if (id) glDeleteProgram(id);
  }

  void bind() const { glUseProgram(id); }

  void uploadUniformBool1(const std::string& name, bool value) const {
    int32_t location = glGetUniformLocation(id, name.c_str());
    assert(location != -1);
    glProgramUniform1i(id, location, value);
  }

  void uploadUniformInt1(const std::string& name, int value) const {
    int32_t location = glGetUniformLocation(id, name.c_str());
    assert(location != -1);
    glProgramUniform1i(id, location, value);
  }

  void uploadUniformInt2(const std::string& name, glm::ivec2 value) const {
    auto location = glGetUniformLocation(id, name.c_str());
    assert(location != -1);
    glProgramUniform2i(id, location, value.x, value.y);
  }

  void uploadUniformFloat1(const std::string& name, float value) const {
    int32_t location = glGetUniformLocation(id, name.c_str());
    assert(location != -1);
    glProgramUniform1f(id, location, value);
  }

  void uploadUniformFloat3(const std::string& name, glm::vec3 value) const {
    int32_t location = glGetUniformLocation(id, name.c_str());
    assert(location != -1);
    glProgramUniform3f(id, location, value.r, value.g, value.b);
  }

  void uploadUniformFloat4(const std::string& name, glm::vec4 value) const {
    int32_t location = glGetUniformLocation(id, name.c_str());
    assert(location != -1);
    glProgramUniform4f(id, location, value.r, value.g, value.b, value.a);
  }

  void uploadUniformMatrix4(const std::string& name, glm::mat4 value) const {
    auto location = glGetUniformLocation(id, name.c_str());
    assert(location != -1);
    glProgramUniformMatrix4fv(id, location, 1, false, &value[0][0]);
  }
};

class Window {
 private:
  GLFWwindow* glfw_window = nullptr;

 public:
  Window(int32_t width, int32_t height, const std::string& title) {
    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) {
      throw std::runtime_error("Failed to initialize GLFW.");
    }

    glfwWindowHint(GLFW_RESIZABLE, false);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfw_window =
        glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

    if (glfw_window == nullptr) {
      glfwTerminate();
      throw std::runtime_error("Failed to create GLFW window.");
    }

    glfwMakeContextCurrent(glfw_window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
      glfwTerminate();
      throw std::runtime_error("Failed to initialize GLAD.");
    }

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(message_callback, 0);

    std::cout << "Vendor: " << glGetString(GL_VENDOR) << "\n";
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << "\n";
    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << "\n";
  }

  Window(Window&& window) noexcept : glfw_window(window.glfw_window) {
    window.glfw_window = nullptr;
  }

  ~Window() {
    glfwDestroyWindow(glfw_window);
    glfwTerminate();
  }

  bool should_close() const { return glfwWindowShouldClose(glfw_window); }

  void swap_buffers() const { glfwSwapBuffers(glfw_window); }

  bool get_key(int key) const { return glfwGetKey(glfw_window, key); }
};

int main() {
  // Defer: glfwTerminate()
  Window window(800, 600, "Lab 1");

  glClearColor(0.5f, 0.0f, 0.0f, 1.0f);
  std::vector<float> triangle = {-0.5f, -0.5f, 0.5f, -0.5f, 0.0f, 0.5f};

  GLuint vertexArrayId;
  glGenVertexArrays(1, &vertexArrayId);
  glBindVertexArray(vertexArrayId);

  Buffer vertex_buffer(BufferType::Vertex, BufferUsage::Static,
                       std::span(triangle));
  vertex_buffer.bind();
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, nullptr);
  glEnableVertexAttribArray(0);

  auto vertex_shader_path = path(ASSETS_DIR) / "vertex.glsl";
  auto fragment_shader_path = path(ASSETS_DIR) / "fragment.glsl";
  Shader shader(vertex_shader_path, fragment_shader_path);
  shader.bind();

  while (!window.should_close()) {
    glfwPollEvents();
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    window.swap_buffers();

    if (window.get_key(GLFW_KEY_ESCAPE) == GLFW_PRESS) break;
  }

  glDisableVertexAttribArray(0);
  // TODO scope guard to delete resources before terminating
  glDeleteVertexArrays(1, &vertexArrayId);

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
