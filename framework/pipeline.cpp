#include "pipeline.h"
#include <format>
#include <ranges>

using namespace framework;

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

Pipeline::Pipeline(
  std::shared_ptr<Shader> shader,
  std::initializer_list<VertexAttribute> vertex_attributes,
  PipelineOptions pipeline_options,
  std::span<BufferLayout> buffer_layouts
) :
  shader(shader),
  pipeline_options(pipeline_options), buffer_meta_data(buffer_layouts.size()) {
  // Calculate stride for each buffer
  for (auto vertex_attribute : vertex_attributes) {
    if (vertex_attribute.buffer_index >= buffer_layouts.size()) {
      throw std::runtime_error("Vertex attribute buffer index is out of bounds."
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
      glGetAttribLocation(shader->id, vertex_attribute.name.c_str());

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

Pipeline::Pipeline(Pipeline &&pipeline) noexcept :
  vertex_array_id(pipeline.vertex_array_id), shader(pipeline.shader),
  pipeline_options(pipeline.pipeline_options),
  buffer_meta_data(std::move(pipeline.buffer_meta_data)) {
  pipeline.vertex_array_id = 0;
}

Pipeline::~Pipeline() {
  if (vertex_array_id) glDeleteVertexArrays(1, &vertex_array_id);
}

void Pipeline::bind() const {
  glBindVertexArray(vertex_array_id);

  auto options = pipeline_options;

  glUseProgram(shader->id);
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

void Pipeline::bind_buffers(
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

void Pipeline::draw(uint32_t elements, uint32_t offset) const {
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
