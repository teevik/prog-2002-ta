#include "buffer.h"

using framework::Buffer;

Buffer::Buffer(Buffer &&object) noexcept : id(object.id), type(object.type) {
  object.id = 0;
}

Buffer::~Buffer() {
  if (id) glDeleteBuffers(1, &id);
}

void Buffer::bind() const {
  glBindBuffer(static_cast<GLenum>(type), id);
}
