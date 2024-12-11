#pragma once

#include "stb_image.h"
#include <cstdint>
#include <string>

namespace framework {
  enum class Filtering { Nearest, Linear, LinearMipmap };

  enum class Wrapping { Repeat };

  class Texture {
  private:
    uint32_t id;
    stbi_uc *pixels;

  public:
    Texture(uint32_t id, stbi_uc *pixels);

    Texture(Texture &&texture) noexcept;

    ~Texture();

    Texture(const Texture &) = delete;

    Texture &operator=(const Texture &) = delete;

    void bind() const;
  };

  Texture loadTexture(
    const std::string &path,
    Filtering filtering = Filtering::LinearMipmap,
    Wrapping wrapping = Wrapping::Repeat
  );

  Texture loadCubemap(
    const std::string &path,
    Filtering filtering = Filtering::LinearMipmap,
    Wrapping wrapping = Wrapping::Repeat
  );
}
