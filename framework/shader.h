#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <filesystem>
#include <string>

namespace framework {
  enum class ShaderType {
    Vertex = GL_VERTEX_SHADER,
    Fragment = GL_FRAGMENT_SHADER,
  };

  // TODO: Shader hot reload?
  struct Shader {
    uint32_t id = 0;

    Shader(
      std::filesystem::path vertex_shader_file,
      std::filesystem::path fragment_shader_file
    );

    Shader(Shader &&shader) noexcept;

    ~Shader();

    void bind() const;

    void uploadUniformBool1(const std::string &name, bool value) const;

    void uploadUniformInt1(const std::string &name, int value) const;

    void uploadUniformInt2(const std::string &name, glm::ivec2 value) const;

    void uploadUniformFloat1(const std::string &name, float value) const;

    void uploadUniformFloat3(const std::string &name, glm::vec3 value) const;

    void uploadUniformFloat4(const std::string &name, glm::vec4 value) const;

    void uploadUniformMatrix4(const std::string &name, glm::mat4 value) const;
  };
}
