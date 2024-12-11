#include "texture.h"
#include <GL/glew.h>
#include <stdexcept>

struct Pixels {
  int width;
  int height;
  stbi_uc *pixels;
};

static Pixels loadPixels(const std::string &path) {
  int width, height, bpp;
  auto pixels = stbi_load(path.c_str(), &width, &height, &bpp, STBI_rgb_alpha);
  if (!pixels) {
    throw std::runtime_error("Failed to load pixels");
  }

  return {.width = width, .height = height, .pixels = pixels};
}

static void applyTextureParameters(
  uint32_t textureId,
  framework::Filtering filtering,
  framework::Wrapping wrapping
) {
  // Wrapping
  int wrappingInt;
  switch (wrapping) {
    case framework::Wrapping::Repeat:
      wrappingInt = GL_REPEAT;
      break;
  }

  glTextureParameteri(textureId, GL_TEXTURE_WRAP_S, wrappingInt);
  glTextureParameteri(textureId, GL_TEXTURE_WRAP_T, wrappingInt);
  glTextureParameteri(textureId, GL_TEXTURE_WRAP_R, wrappingInt);

  // Filtering
  switch (filtering) {
    case framework::Filtering::Nearest:
      glTextureParameteri(textureId, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTextureParameteri(textureId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

      break;

    case framework::Filtering::Linear:
      glTextureParameteri(textureId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTextureParameteri(textureId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

      break;

    case framework::Filtering::LinearMipmap:
      glGenerateTextureMipmap(textureId);

      glTextureParameteri(
        textureId, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR
      );
      glTextureParameteri(textureId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

      break;
  }
}

namespace framework {
  Texture::Texture(uint32_t id, stbi_uc *pixels) : id(id), pixels(pixels) {}

  Texture::Texture(Texture &&texture) noexcept :
    id(texture.id), pixels(texture.pixels) {
    texture.id = 0;
    texture.pixels = nullptr;
  }

  Texture::~Texture() {
    if (id) glDeleteTextures(1, &id);
    if (pixels) stbi_image_free((void *)pixels);
  }

  void Texture::bind() const {
    glBindTextureUnit(0, id);
  }

  Texture loadTexture(
    const std::string &path, Filtering filtering, Wrapping wrapping
  ) {
    auto [imageWidth, imageHeight, pixels] = loadPixels(path);

    uint32_t textureId;
    glCreateTextures(GL_TEXTURE_2D, 1, &textureId);

    glTextureStorage2D(textureId, 1, GL_RGBA8, imageWidth, imageHeight);
    glTextureSubImage2D(
      textureId,
      0,
      0,
      0,
      imageWidth,
      imageHeight,
      GL_RGBA,
      GL_UNSIGNED_BYTE,
      pixels
    );

    applyTextureParameters(textureId, filtering, wrapping);

    return {textureId, pixels};
  }

  Texture loadCubemap(
    const std::string &path, Filtering filtering, Wrapping wrapping
  ) {
    auto [imageWidth, imageHeight, pixels] = loadPixels(path);

    uint32_t textureId;
    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &textureId);

    glTextureStorage2D(textureId, 1, GL_RGBA8, imageWidth, imageHeight);
    for (int i = 0; i < 6; ++i) {
      glTextureSubImage3D(
        textureId,
        0,
        0,
        0,
        i,
        imageWidth,
        imageHeight,
        1,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        pixels
      );
    }

    applyTextureParameters(textureId, filtering, wrapping);

    return {textureId, pixels};
  }
}
