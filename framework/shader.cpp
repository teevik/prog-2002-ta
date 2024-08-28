#include "shader.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>

using namespace framework;

std::string read_file_to_string(const std::filesystem::path path) {
  std::ifstream file(path);
  std::string content(
    (std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>())
  );

  return content;
}

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

Shader::Shader(
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

Shader::Shader(Shader &&shader) noexcept : id(shader.id) {
  shader.id = 0;
}

Shader::~Shader() {
  if (id) glDeleteProgram(id);
}

void Shader::bind() const {
  glUseProgram(id);
}

void Shader::uploadUniformBool1(const std::string &name, bool value) const {
  int32_t location = glGetUniformLocation(id, name.c_str());
  assert(location != -1);
  glProgramUniform1i(id, location, value);
}

void Shader::uploadUniformInt1(const std::string &name, int value) const {
  int32_t location = glGetUniformLocation(id, name.c_str());
  assert(location != -1);
  glProgramUniform1i(id, location, value);
}

void Shader::uploadUniformInt2(const std::string &name, glm::ivec2 value)
  const {
  auto location = glGetUniformLocation(id, name.c_str());
  assert(location != -1);
  glProgramUniform2i(id, location, value.x, value.y);
}

void Shader::uploadUniformFloat1(const std::string &name, float value) const {
  int32_t location = glGetUniformLocation(id, name.c_str());
  assert(location != -1);
  glProgramUniform1f(id, location, value);
}

void Shader::uploadUniformFloat3(const std::string &name, glm::vec3 value)
  const {
  int32_t location = glGetUniformLocation(id, name.c_str());
  assert(location != -1);
  glProgramUniform3f(id, location, value.r, value.g, value.b);
}

void Shader::uploadUniformFloat4(const std::string &name, glm::vec4 value)
  const {
  int32_t location = glGetUniformLocation(id, name.c_str());
  assert(location != -1);
  glProgramUniform4f(id, location, value.r, value.g, value.b, value.a);
}

void Shader::uploadUniformMatrix4(const std::string &name, glm::mat4 value)
  const {
  auto location = glGetUniformLocation(id, name.c_str());
  assert(location != -1);
  glProgramUniformMatrix4fv(id, location, 1, false, &value[0][0]);
}
