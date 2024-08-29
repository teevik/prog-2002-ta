#pragma once

#include <GL/glew.h>
#include <span>

namespace framework {
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
    Buffer(Buffer &&object) noexcept;

    ~Buffer();

    void bind() const;
  };
}