#include "framework/ranges.h"
#include "framework/shapes.h"
#include <framework/buffer.h>
#include <framework/pipeline.h>
#include <framework/shader.h>
#include <framework/window.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cstdlib>
#include <filesystem>
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
};

const glm::ivec2 BOARD_TILES = {10, 10};

int main(int argc, char *argv[]) {
  path program_path(argv[0]);
  path program_folder = program_path.parent_path();
  path assets_folder = program_folder / "assets";

  Window window(800, 600, "Lab 2", false);

  auto grid = shapes::grid(BOARD_TILES.x, BOARD_TILES.y);

  // clang-format off
  auto vertices = grid.vertices
    | views::transform([](auto v) { return Vertex{.position = v.position.xy()}; })
    | to<std::vector<Vertex>>();
  // clang-format on

  auto indices = grid.indices;

  Buffer vertex_buffer(
    BufferType::Vertex, BufferUsage::Static, std::span(vertices)
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
  };
  Pipeline pipeline(shader, attributes);

  while (!window.should_close()) {
    auto fov = glm::radians(45.0f);
    auto aspect_ratio = window.get_aspect_ratio();
    auto z_near = 1.f;
    auto z_far = -10.f;

    auto projection_matrix = glm::perspective(fov, aspect_ratio, z_near, z_far);
    auto view_matrix = glm::lookAt(
      glm::vec3(0.0f, -1.0f, 0.5f),
      glm::vec3(0.0f, 0.0f, 0.0f),
      glm::vec3(0.0f, 1.0f, 0.0f)
    );

    auto model_matrix = glm::mat4(1.0f);

    // TODO camera
    window.begin_default_pass(Clear{.color = array{0.5f, 0.0f, 0.0f, 1.0f}});

    shader->uploadUniformMatrix4("projection_matrix", projection_matrix);
    shader->uploadUniformMatrix4("view_matrix", view_matrix);
    shader->uploadUniformMatrix4("model_matrix", model_matrix);

    shader->uploadUniformInt2("board_tiles", BOARD_TILES);

    pipeline.bind();
    pipeline.bind_buffers({std::ref(vertex_buffer)}, index_buffer);
    pipeline.draw(indices.size());

    window.commit_frame();

    if (window.get_key(GLFW_KEY_ESCAPE) == GLFW_PRESS) break;
  }

  return EXIT_SUCCESS;
}
