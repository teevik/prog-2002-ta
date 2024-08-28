#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <glm/glm.hpp>
#include <ios>
#include <iostream>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <vector>

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

enum class CullFace {
  Nothing = GL_NONE,
  Front = GL_FRONT,
  Back = GL_BACK,
};

enum class FrontFaceOrder {
  Clockwise = GL_CW,
  CounterClockwise = GL_CCW,
};

enum class DepthTest {
  Never = GL_NEVER,
  Less = GL_LESS,
  Equal = GL_EQUAL,
  LessOrEqual = GL_LEQUAL,
  Greater = GL_GREATER,
  NotEqual = GL_NOTEQUAL,
  GreaterOrEqual = GL_GEQUAL,
  Always = GL_ALWAYS,
};

enum class Equation {
  Add = GL_FUNC_ADD,
  Subtract = GL_FUNC_SUBTRACT,
  ReverseSubtract = GL_FUNC_REVERSE_SUBTRACT,
};

enum class BlendFactor {
  Zero = GL_ZERO,
  One = GL_ONE,
  SourceColor = GL_SRC_COLOR,
  OneMinusSourceColor = GL_ONE_MINUS_SRC_COLOR,
  DestinationColor = GL_DST_COLOR,
  OneMinusDestinationColor = GL_ONE_MINUS_DST_COLOR,
  SourceAlpha = GL_SRC_ALPHA,
  OneMinusSourceAlpha = GL_ONE_MINUS_SRC_ALPHA,
  DestinationAlpha = GL_DST_ALPHA,
  OneMinusDestinationAlpha = GL_ONE_MINUS_DST_ALPHA,
};

struct BlendState {
  Equation equation;
  BlendFactor source_factor;
  BlendFactor destination_factor;
};

enum class StencilOperation {
  Keep = GL_KEEP,
  Zero = GL_ZERO,
  Replace = GL_REPLACE,
  IncrementClamp = GL_INCR,
  IncrementWrap = GL_INCR_WRAP,
  DecrementClamp = GL_DECR,
  DecrementWrap = GL_DECR_WRAP,
  Invert = GL_INVERT,
};

enum class CompareFunction {
  Never = GL_NEVER,
  Less = GL_LESS,
  Equal = GL_EQUAL,
  LessOrEqual = GL_LEQUAL,
  Greater = GL_GREATER,
  NotEqual = GL_NOTEQUAL,
  GreaterOrEqual = GL_GEQUAL,
  Always = GL_ALWAYS,
};

struct StencilFaceState {
  StencilOperation fail_operation;
  StencilOperation depth_fail_operation;
  StencilOperation pass_operation;
  CompareFunction test_function;
  int32_t test_reference;
  uint32_t test_mask;
  uint32_t write_mask;
};

struct StencilState {
  StencilFaceState front_face;
  StencilFaceState back_face;
};

enum class PrimitiveType {
  Triangles = GL_TRIANGLES,
  Lines = GL_LINES,
  Points = GL_POINTS,
};

struct PipelineOptions {
  CullFace cull_face = CullFace::Nothing;
  FrontFaceOrder front_face_order = FrontFaceOrder::CounterClockwise;
  DepthTest depth_test = DepthTest::Always;
  bool depth_write = false;
  std::optional<std::array<float, 2>> depth_write_offset = std::nullopt;
  std::optional<BlendState> color_blend = std::nullopt;
  std::optional<BlendState> alpha_blend = std::nullopt;
  std::optional<StencilState> stencil_test = std::nullopt;
  std::array<bool, 4> color_mask = {true, true, true, true};
  PrimitiveType primitive_type = PrimitiveType::Triangles;
};

struct BufferMetaData {
  uint32_t stride;
  uint32_t offset;
};

enum class VertexFormat {
  Float1,
  Float2,
  Float3,
  Float4,
  Byte1,
  Byte2,
  Byte3,
  Byte4,
  Short1,
  Short2,
  Short3,
  Short4,
  Int1,
  Int2,
  Int3,
  Int4,
  Mat4,
};

uint32_t bytes_of(VertexFormat vertex_format) {
  using enum VertexFormat;

  switch (vertex_format) {
    case Float1:
      return 4;
    case Float2:
      return 8;
    case Float3:
      return 12;
    case Float4:
      return 16;
    case Byte1:
      return 1;
    case Byte2:
      return 2;
    case Byte3:
      return 3;
    case Byte4:
      return 4;
    case Short1:
      return 2;
    case Short2:
      return 4;
    case Short3:
      return 6;
    case Short4:
      return 8;
    case Int1:
      return 4;
    case Int2:
      return 8;
    case Int3:
      return 12;
    case Int4:
      return 16;
    case Mat4:
      return 64;
  }
}

uint32_t components_of(VertexFormat vertex_format) {
  using enum VertexFormat;

  switch (vertex_format) {
    case Float1:
    case Byte1:
    case Short1:
    case Int1:
      return 1;

    case Float2:
    case Byte2:
    case Short2:
    case Int2:
      return 2;

    case Float3:
    case Byte3:
    case Short3:
    case Int3:
      return 3;

    case Float4:
    case Byte4:
    case Short4:
    case Int4:
      return 4;

    case Mat4:
      return 16;
  }
}

struct VertexAttribute {
  VertexFormat vertex_format;
  uint32_t buffer_index;
};

enum class VertexStep {
  PerVertex,
  PerInstance,
};

struct BufferLayout {
  uint32_t stride = 0;
  VertexStep step = VertexStep::PerVertex;
  uint32_t step_rate = 1;
};

class Pipeline {
 private:
  uint32_t vertex_array_id = 0;
  Shader shader;
  PipelineOptions pipeline_options;
  std::vector<BufferMetaData> buffer_meta_data;

  // constexpr const static std::array<BufferLayout, 1> DEFAULT_BUFFER_LAYOUT =
  // {

  inline static std::array<BufferLayout, 1> DEFAULT_BUFFER_LAYOUT = {{}};

 public:
  Pipeline(
      std::span<VertexAttribute> vertex_attributes, Shader&& shader,
      PipelineOptions pipeline_options = {},
      std::span<BufferLayout> buffer_layouts = std::span(DEFAULT_BUFFER_LAYOUT))
      : shader(std::move(shader)) {}
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

    auto glew_error = glewInit();
    if (glew_error != GLEW_OK) {
      auto error_message =
          reinterpret_cast<const char*>(glewGetErrorString(glew_error));

      glfwTerminate();
      throw std::runtime_error(std::string("Failed to initialize GLEW: ") +
                               error_message);
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

int main(int argc, char* argv[]) {
  path program_path(argv[0]);
  path program_folder = program_path.parent_path();
  path assets_folder = program_folder / "assets";

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

  auto vertex_shader_path = assets_folder / "vertex.glsl";
  auto fragment_shader_path = assets_folder / "fragment.glsl";
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
