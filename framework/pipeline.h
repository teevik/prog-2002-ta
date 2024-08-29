#pragma once

#include "framework/buffer.h"
#include <GL/glew.h>
#include <framework/shader.h>
#include <array>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace framework {
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
    );

    Pipeline(Pipeline &&pipeline) noexcept;

    ~Pipeline();

    void bind() const;

    void bind_buffers(
      std::initializer_list<std::reference_wrapper<Buffer>> vertex_buffers,
      const Buffer &index_buffer
    ) const;

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
}
