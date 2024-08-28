#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <ios>
#include <iostream>
#include <memory>
#include <optional>
#include <ranges>
#include <span>
#include <string>
#include <vector>

using std::filesystem::path;

enum class BufferType {
  Vertex = GL_ARRAY_BUFFER,
  Index = GL_ELEMENT_ARRAY_BUFFER,
};

enum class BufferUsage {
  Static = GL_STATIC_DRAW,
  Dynamic = GL_DYNAMIC_DRAW,
  Stream = GL_STREAM_DRAW,
};

struct Buffer {
  uint32_t id = 0;
  BufferType type;

  template <typename T>
  Buffer(BufferType type, BufferUsage usage, std::span<T> data) : type(type) {
    glGenBuffers(1, &id);

    glBindBuffer(static_cast<GLenum>(type), id);
    glBufferData(
      static_cast<GLenum>(type),
      sizeof(T) * data.size(),
      data.data(),
      static_cast<GLenum>(usage)
    );
  };

  // Move constructor
  Buffer(Buffer &&object) noexcept : id(object.id), type(object.type) {
    object.id = 0;
  }

  ~Buffer() {
    if (id) glDeleteBuffers(1, &id);
  }

  void bind() const {
    glBindBuffer(static_cast<GLenum>(type), id);
  }
};

std::string read_file_to_string(const std::filesystem::path path) {
  std::ifstream file(path);
  std::string content(
    (std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>())
  );

  return content;
}

enum class ShaderType {
  Vertex = GL_VERTEX_SHADER,
  Fragment = GL_FRAGMENT_SHADER,
};

std::optional<uint32_t> compile_shader(
  const std::string &source, ShaderType shaderType
) {
  auto shader_id = glCreateShader(static_cast<GLenum>(shaderType));

  const char *raw_source = source.c_str();
  glShaderSource(shader_id, 1, &raw_source, nullptr);
  glCompileShader(shader_id);

  int32_t shader_did_compile;
  glGetShaderiv(shader_id, GL_COMPILE_STATUS, &shader_did_compile);

  if (!shader_did_compile) {
    int32_t errorLength;
    glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &errorLength);

    auto errorMessage = std::make_unique<char[]>(errorLength);
    glGetShaderInfoLog(
      shader_id, errorLength, &errorLength, errorMessage.get()
    );

    std::cerr << "Failed to compile shader!" << std::endl;
    std::cerr << errorMessage.get() << std::endl;

    glDeleteShader(shader_id);

    return std::nullopt;
  }

  return shader_id;
};

// TODO: Shader hot reload?
struct Shader {
  uint32_t id = 0;

  Shader(
    std::filesystem::path vertex_shader_file,
    std::filesystem::path fragment_shader_file
  ) {
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

  Shader(Shader &&shader) noexcept : id(shader.id) {
    shader.id = 0;
  }

  ~Shader() {
    if (id) glDeleteProgram(id);
  }

  void bind() const {
    glUseProgram(id);
  }

  void uploadUniformBool1(const std::string &name, bool value) const {
    int32_t location = glGetUniformLocation(id, name.c_str());
    assert(location != -1);
    glProgramUniform1i(id, location, value);
  }

  void uploadUniformInt1(const std::string &name, int value) const {
    int32_t location = glGetUniformLocation(id, name.c_str());
    assert(location != -1);
    glProgramUniform1i(id, location, value);
  }

  void uploadUniformInt2(const std::string &name, glm::ivec2 value) const {
    auto location = glGetUniformLocation(id, name.c_str());
    assert(location != -1);
    glProgramUniform2i(id, location, value.x, value.y);
  }

  void uploadUniformFloat1(const std::string &name, float value) const {
    int32_t location = glGetUniformLocation(id, name.c_str());
    assert(location != -1);
    glProgramUniform1f(id, location, value);
  }

  void uploadUniformFloat3(const std::string &name, glm::vec3 value) const {
    int32_t location = glGetUniformLocation(id, name.c_str());
    assert(location != -1);
    glProgramUniform3f(id, location, value.r, value.g, value.b);
  }

  void uploadUniformFloat4(const std::string &name, glm::vec4 value) const {
    int32_t location = glGetUniformLocation(id, name.c_str());
    assert(location != -1);
    glProgramUniform4f(id, location, value.r, value.g, value.b, value.a);
  }

  void uploadUniformMatrix4(const std::string &name, glm::mat4 value) const {
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

uint32_t gl_type_of(VertexFormat vertex_format) {
  using enum VertexFormat;

  switch (vertex_format) {
    case Float1:
    case Float2:
    case Float3:
    case Float4:
      return GL_FLOAT;

    case Byte1:
    case Byte2:
    case Byte3:
    case Byte4:
      return GL_UNSIGNED_BYTE;

    case Short1:
    case Short2:
    case Short3:
    case Short4:
      return GL_UNSIGNED_SHORT;

    case Int1:
    case Int2:
    case Int3:
    case Int4:
      return GL_UNSIGNED_INT;

    case Mat4:
      return GL_FLOAT;
  }
}

uint32_t attribute_count_of(VertexFormat vertex_format) {
  if (vertex_format == VertexFormat::Mat4) {
    return 4;
  } else {
    return 1;
  }
}

struct VertexAttribute {
  std::string name;
  VertexFormat format;
  uint32_t buffer_index = 0;
};

enum class VertexStep {
  PerVertex,
  PerInstance,
};

struct BufferLayout {
  std::optional<uint32_t> stride = std::nullopt;
  VertexStep step = VertexStep::PerVertex;
  uint32_t step_rate = 1;
};

struct VertexAttributeLayout {
  uint32_t attribute_location;
  uint32_t size;
  uint32_t type;
  uint32_t offset;
  uint32_t stride;
  uint32_t buffer_index;
  uint32_t divisor;
};

struct Pipeline {
  uint32_t vertex_array_id = 0;
  Shader shader;
  PipelineOptions pipeline_options;
  std::vector<BufferMetaData> buffer_meta_data;

  inline static std::array<BufferLayout, 1> DEFAULT_BUFFER_LAYOUT = {{}};

  Pipeline(
    Shader &&moved_shader,
    std::initializer_list<VertexAttribute> vertex_attributes,
    PipelineOptions pipeline_options = {},
    std::span<BufferLayout> buffer_layouts = std::span(DEFAULT_BUFFER_LAYOUT)
  ) :
    shader(std::move(moved_shader)),
    pipeline_options(pipeline_options),
    buffer_meta_data(buffer_layouts.size()) {
    // Calculate stride for each buffer
    for (auto vertex_attribute : vertex_attributes) {
      if (vertex_attribute.buffer_index >= buffer_layouts.size()) {
        throw std::runtime_error(
          "Vertex attribute buffer index is out of bounds."
        );
      }

      auto layout = buffer_layouts[vertex_attribute.buffer_index];
      auto buffer = &buffer_meta_data[vertex_attribute.buffer_index];

      if (layout.stride.has_value()) {
        buffer->stride = layout.stride.value();
      } else {
        buffer->stride += bytes_of(vertex_attribute.format);
      }
    }

    uint32_t attributes_count = 0;
    for (auto vertex_attribute : vertex_attributes) {
      attributes_count += attribute_count_of(vertex_attribute.format);
    }

    std::vector<VertexAttributeLayout> vertex_layout(attributes_count);

    for (auto vertex_attribute : vertex_attributes) {
      auto layout = buffer_layouts[vertex_attribute.buffer_index];
      auto buffer = &buffer_meta_data[vertex_attribute.buffer_index];

      auto attribute_location =
        glGetAttribLocation(shader.id, vertex_attribute.name.c_str());

      if (attribute_location == -1) {
        throw std::runtime_error(
          std::format("Vertex attribute {} not found.", vertex_attribute.name)
        );
      }

      uint32_t divisor;
      if (layout.step == VertexStep::PerVertex) {
        divisor = 0;
      } else {
        divisor = layout.step_rate;
      }

      auto attribute_count = attribute_count_of(vertex_attribute.format);

      for (uint32_t i = 0; i < attribute_count; i++) {
        auto offset_location = attribute_location + i;

        if (offset_location >= vertex_layout.size()) {
          throw std::runtime_error("Vertex layout index out of bounds.");
        }

        auto attribute = VertexAttributeLayout{
          .attribute_location = offset_location,
          .size = components_of(vertex_attribute.format),
          .type = gl_type_of(vertex_attribute.format),
          .offset = buffer->offset,
          .stride = buffer->stride,
          .buffer_index = vertex_attribute.buffer_index,
          .divisor = divisor,
        };

        vertex_layout[offset_location] = attribute;
        buffer->offset += bytes_of(vertex_attribute.format);
      }
    }

    glCreateVertexArrays(1, &vertex_array_id);

    for (uint32_t attribute_index = 0; attribute_index < vertex_layout.size();
         attribute_index++) {
      auto layout = vertex_layout[attribute_index];

      glEnableVertexArrayAttrib(vertex_array_id, attribute_index);
      glVertexArrayAttribBinding(
        vertex_array_id, attribute_index, layout.buffer_index
      );

      switch (layout.type) {
        case GL_INT:
        case GL_UNSIGNED_INT:
        case GL_UNSIGNED_SHORT:
        case GL_UNSIGNED_BYTE:
        case GL_BYTE: {
          glVertexArrayAttribIFormat(
            vertex_array_id,
            attribute_index,
            layout.size,
            layout.type,
            layout.offset
          );
          break;
        }

        default: {
          glVertexArrayAttribFormat(
            vertex_array_id,
            attribute_index,
            layout.size,
            layout.type,
            GL_FALSE,
            layout.offset
          );
          break;
        }
      }

      glVertexArrayBindingDivisor(
        vertex_array_id, layout.buffer_index, layout.divisor
      );
    }
  }

  Pipeline(Pipeline &&pipeline) noexcept :
    vertex_array_id(pipeline.vertex_array_id),
    shader(std::move(pipeline.shader)),
    pipeline_options(pipeline.pipeline_options),
    buffer_meta_data(std::move(pipeline.buffer_meta_data)) {
    pipeline.vertex_array_id = 0;
  }

  ~Pipeline() {
    if (vertex_array_id) glDeleteVertexArrays(1, &vertex_array_id);
  }

  void bind() const {
    glBindVertexArray(vertex_array_id);

    auto options = pipeline_options;

    glUseProgram(shader.id);
    glEnable(GL_SCISSOR_TEST);

    if (options.depth_write) {
      glEnable(GL_DEPTH_TEST);
      glDepthFunc(static_cast<GLenum>(options.depth_test));
    } else {
      glDisable(GL_DEPTH_TEST);
    }

    glFrontFace(static_cast<GLenum>(options.front_face_order));

    switch (options.cull_face) {
      case CullFace::Nothing: {
        glDisable(GL_CULL_FACE);
        break;
      }

      case CullFace::Front: {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        break;
      }

      case CullFace::Back: {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        break;
      }
    }

    if (options.color_blend.has_value()) {
      glEnable(GL_BLEND);

      auto color_blend = options.color_blend.value();

      if (options.alpha_blend.has_value()) {
        auto alpha_blend = options.alpha_blend.value();

        glBlendFuncSeparate(
          static_cast<GLenum>(color_blend.source_factor),
          static_cast<GLenum>(color_blend.destination_factor),
          static_cast<GLenum>(alpha_blend.source_factor),
          static_cast<GLenum>(alpha_blend.destination_factor)
        );

        glBlendEquationSeparate(
          static_cast<GLenum>(color_blend.equation),
          static_cast<GLenum>(alpha_blend.equation)
        );
      } else {
        glBlendFunc(
          static_cast<GLenum>(color_blend.source_factor),
          static_cast<GLenum>(color_blend.destination_factor)
        );
        glBlendEquationSeparate(
          static_cast<GLenum>(color_blend.equation),
          static_cast<GLenum>(color_blend.equation)
        );
      }
    } else {
      glDisable(GL_BLEND);
    }

    if (options.stencil_test.has_value()) {
      glEnable(GL_STENCIL_TEST);

      auto stencil_test = options.stencil_test.value();

      auto front = stencil_test.front_face;
      glStencilOpSeparate(
        GL_FRONT,
        static_cast<GLenum>(front.fail_operation),
        static_cast<GLenum>(front.depth_fail_operation),
        static_cast<GLenum>(front.pass_operation)
      );
      glStencilFuncSeparate(
        GL_FRONT,
        static_cast<GLenum>(front.test_function),
        front.test_reference,
        front.test_mask
      );
      glStencilMaskSeparate(GL_FRONT, front.write_mask);

      auto back = stencil_test.back_face;
      glStencilOpSeparate(
        GL_BACK,
        static_cast<GLenum>(back.fail_operation),
        static_cast<GLenum>(back.depth_fail_operation),
        static_cast<GLenum>(back.pass_operation)
      );
      glStencilFuncSeparate(
        GL_BACK,
        static_cast<GLenum>(back.test_function),
        back.test_reference,
        back.test_mask
      );
      glStencilMaskSeparate(GL_BACK, back.write_mask);
    } else {
      glDisable(GL_STENCIL_TEST);
    }

    auto color_mask = options.color_mask;
    glColorMask(color_mask[0], color_mask[1], color_mask[2], color_mask[3]);
  }

  void bind_buffers(
    std::initializer_list<std::reference_wrapper<Buffer>> vertex_buffers,
    const Buffer &index_buffer
  ) const {
    for (auto const [index, vertex_buffer] :
         std::views::enumerate(vertex_buffers)) {
      glVertexArrayVertexBuffer(
        vertex_array_id,
        index,
        vertex_buffer.get().id,
        0,
        buffer_meta_data[index].stride
      );
    }

    glVertexArrayElementBuffer(vertex_array_id, index_buffer.id);
  }

  void draw(uint32_t elements, uint32_t offset = 0) const {
    auto options = pipeline_options;
    // TODO different index types?
    auto index_type = GL_UNSIGNED_INT;
    auto index_size = 4;

    glDrawElements(
      static_cast<GLenum>(options.primitive_type),
      elements,
      index_type,
      reinterpret_cast<void *>(offset * index_size)
    );
  }
};

struct Window {
  GLFWwindow *glfw_window = nullptr;

  Window(int32_t width, int32_t height, const std::string &title) {
    auto error_callback = [](int code, const char *description) {
      std::cerr << "GLFW Error (0x" << std::hex << code << "): " << description
                << "\n";
    };
    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) {
      throw std::runtime_error("Failed to initialize GLFW.");
    }

    glfwWindowHint(GLFW_RESIZABLE, true);
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
        reinterpret_cast<const char *>(glewGetErrorString(glew_error));

      glfwTerminate();
      throw std::runtime_error(
        std::string("Failed to initialize GLEW: ") + error_message
      );
    }

    auto message_callback = [](
                              GLenum source,
                              GLenum type,
                              GLuint id,
                              GLenum severity,
                              GLsizei length,
                              const GLchar *message,
                              const void *userParam
                            ) {
      std::cerr << "OpenGL Callback: "
                << (type == GL_DEBUG_TYPE_ERROR ? "** ERROR **" : "")
                << " Type: 0x" << type << ", Severity: 0x" << severity
                << ", Message: " << message << "\n";
    };

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(message_callback, 0);

    auto framebuffer_size_callback =
      [](GLFWwindow *window, int width, int height) {
        glViewport(0, 0, width, height);
        glScissor(0, 0, width, height);
      };

    glfwSetFramebufferSizeCallback(glfw_window, framebuffer_size_callback);
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << "\n";
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << "\n";
    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << "\n";
  }

  Window(Window &&window) noexcept : glfw_window(window.glfw_window) {
    window.glfw_window = nullptr;
  }

  ~Window() {
    if (glfw_window) {
      glfwDestroyWindow(glfw_window);
      glfwTerminate();
    }
  }

  bool should_close() const {
    return glfwWindowShouldClose(glfw_window);
  }

  void swap_buffers() const {
    glfwSwapBuffers(glfw_window);
  }

  bool get_key(int key) const {
    return glfwGetKey(glfw_window, key);
  }
};

int main(int argc, char *argv[]) {
  path program_path(argv[0]);
  path program_folder = program_path.parent_path();
  path assets_folder = program_folder / "assets";

  Window window(800, 600, "Lab 1");

  glClearColor(0.5f, 0.0f, 0.0f, 1.0f);
  std::vector<glm::vec2> vertices = {
    {-0.5f, -0.5f}, {0.5f, -0.5f}, {0.0f, 0.5f}
  };
  std::vector<uint32_t> indices = {0, 1, 2};

  auto vertex_shader_path = assets_folder / "vertex.glsl";
  auto fragment_shader_path = assets_folder / "fragment.glsl";
  Shader shader(vertex_shader_path, fragment_shader_path);

  Pipeline pipeline(
    std::move(shader),
    {
      VertexAttribute{.name = "position", .format = VertexFormat::Float2},
    }
  );

  Buffer vertex_buffer(
    BufferType::Vertex, BufferUsage::Static, std::span(vertices)
  );
  Buffer index_buffer(
    BufferType::Index, BufferUsage::Static, std::span(indices)
  );

  while (!window.should_close()) {
    glfwPollEvents();
    glClear(GL_COLOR_BUFFER_BIT);

    pipeline.bind();
    pipeline.bind_buffers({std::ref(vertex_buffer)}, index_buffer);
    pipeline.draw(indices.size());

    window.swap_buffers();

    if (window.get_key(GLFW_KEY_ESCAPE) == GLFW_PRESS) break;
  }

  return EXIT_SUCCESS;
}
