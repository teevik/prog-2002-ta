#include "framework/ranges.h"
#include "framework/shapes.h"
#include <framework/buffer.h>
#include <framework/pipeline.h>
#include <framework/shader.h>
#include <framework/window.h>
#include <glm/glm.hpp>
#include <glm/gtx/color_space.hpp>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <ranges>
#include <span>
#include <string>
#include <vector>

using namespace framework;
using std::array;
using std::string;
using std::filesystem::path;
namespace views = std::views;

struct Vertex {
  glm::vec2 position;
  glm::vec3 color;
};

float repeat(float t, float length) {
  return glm::clamp(t - glm::floor(t / length) * length, 0.f, length);
};

int main(int argc, char *argv[]) {
  path program_path(argv[0]);
  path program_folder = program_path.parent_path();
  path assets_folder = program_folder / "assets";

  Window window(800, 600, "Lab 1", false);

  // clang-format off
  auto vertices = shapes::triangle.vertices 
    | views::enumerate
    | views::transform([](auto x) {
        auto [index, v] = x;
        auto red = index % 3 == 0 ? 1.0f : 0.0f;
        auto green = index % 3 == 1 ? 1.0f : 0.0f;
        auto blue = index % 3 == 2 ? 1.0f : 0.0f;
        
        return Vertex{.position = v.position.xy(), .color={red,green,blue}};
    })
    | to<std::vector<Vertex>>();
  // clang-format on

  auto indices = shapes::triangle.indices;

  Buffer vertex_buffer(
    BufferType::Vertex, BufferUsage::Dynamic, std::span(vertices)
  );
  Buffer index_buffer(
    BufferType::Index, BufferUsage::Static, std::span(indices)
  );

  auto vertex_shader_path = assets_folder / "vertex.glsl";
  auto fragment_shader_path = assets_folder / "fragment.glsl";

  auto shader =
    std::make_shared<Shader>(vertex_shader_path, fragment_shader_path);

  auto attributes = {
    VertexAttribute{.name = "position", .format = VertexFormat::Float2},
    VertexAttribute{.name = "color", .format = VertexFormat::Float3},
  };
  Pipeline pipeline(shader, attributes);

  while (!window.should_close()) {
    auto time = window.time();

    auto amount = vertices.size();

    // rainbow based on time, each vertex
    for (auto const [index, vertex] : views::enumerate(vertices)) {
      // auto hue = (float)index * (360.f / amount) + time * 50.f;

      vertex.color = glm::rgbColor(glm::vec3{hue, 1.f, 1.f});
      // auto red = index % 3 == 0 ? 1.0f : 0.0f;
      // auto green = index % 3 == 1 ? 1.0f : 0.0f;
      // auto blue = index % 3 == 2 ? 1.0f : 0.0f;

      // vertex.color = {
      //   (glm::cos(red + (time / 1.f)) + 1.f) / 2.f,
      //   (glm::cos(green + (time / 1.f)) + 1.f) / 2.f,
      //   (glm::cos(blue + (time / 1.f)) + 1.f) / 2.f
      // };
    }
    vertex_buffer.updateData(std::span(vertices));

    window.begin_default_pass(Clear{.color = array{0.5f, 0.0f, 0.0f, 1.0f}});

    pipeline.bind();
    pipeline.bind_buffers({std::ref(vertex_buffer)}, index_buffer);
    pipeline.draw(indices.size());

    window.commit_frame();

    if (window.get_key(GLFW_KEY_ESCAPE) == GLFW_PRESS) break;
  }

  return EXIT_SUCCESS;
}
