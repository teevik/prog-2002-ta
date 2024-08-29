#include <framework/buffer.h>
#include <framework/pipeline.h>
#include <framework/shader.h>
#include <framework/window.h>
#include <glm/glm.hpp>
#include <cstdlib>
#include <filesystem>
#include <span>
#include <string>
#include <vector>

using namespace framework;
using std::array;
using std::string;
using std::filesystem::path;

int main(int argc, char *argv[]) {
  path program_path(argv[0]);
  path program_folder = program_path.parent_path();
  path assets_folder = program_folder / "assets";

  Window window(800, 600, "Lab 1", false);

  std::vector<glm::vec2> vertices = {
    {-0.5f, -0.5f}, {0.5f, -0.5f}, {0.0f, 0.5f}
  };
  std::vector<uint32_t> indices = {0, 1, 2};

  auto vertex_shader_path = assets_folder / "vertex.glsl";
  auto fragment_shader_path = assets_folder / "fragment.glsl";
  Shader shader(vertex_shader_path, fragment_shader_path);

  Pipeline pipeline(
    std::move(shader),
    {
      VertexAttribute{.name = "position", .format = VertexFormat::Float2},
    }
  );

  Buffer vertex_buffer(
    BufferType::Vertex, BufferUsage::Static, std::span(vertices)
  );
  Buffer index_buffer(
    BufferType::Index, BufferUsage::Static, std::span(indices)
  );

  while (!window.should_close()) {
    window.begin_default_pass(Clear{.color = array{0.5f, 0.0f, 0.0f, 1.0f}});

    pipeline.bind();
    pipeline.bind_buffers({std::ref(vertex_buffer)}, index_buffer);
    pipeline.draw(indices.size());

    window.commit_frame();

    if (window.get_key(GLFW_KEY_ESCAPE) == GLFW_PRESS) break;
  }

  return EXIT_SUCCESS;
}
