#pragma once

#include <glm/glm.hpp>
#include <vector>

namespace framework::shapes {
  struct Vertex {
    glm::vec3 position;
    glm::vec2 texture_coordinate;
  };

  struct Bindings {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
  };

  const Bindings triangle{
    .vertices =
      {{.position = {-0.5f, -0.5f, 0.0f}, .texture_coordinate = {0.0f, 0.0f}},
       {.position = {0.5f, -0.5f, 0.0f}, .texture_coordinate = {1.0f, 0.0f}},
       {.position = {0.0f, 0.5f, 0.0f}, .texture_coordinate = {0.5f, 1.0f}}},
    .indices = {0, 1, 2}
  };

  const Bindings quad = {
    .vertices =
      {{.position = {-0.5f, -0.5f, 0.0f}, .texture_coordinate = {0.0f, 0.0f}},
       {.position = {0.5f, -0.5f, 0.0f}, .texture_coordinate = {1.0f, 0.0f}},
       {.position = {0.5f, 0.5f, 0.0f}, .texture_coordinate = {1.0f, 1.0f}},
       {.position = {-0.5f, 0.5f, 0.0f}, .texture_coordinate = {0.0f, 1.0f}}},
    .indices = {0, 1, 2, 2, 3, 0}
  };

  // unit grid from centered at origin from -0.5 to 0.5
  Bindings grid(uint32_t rows, uint32_t columns) {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    for (uint32_t i = 0; i < rows; i++) {
      for (uint32_t j = 0; j < columns; j++) {
        float x = (j / (columns - 1.0f)) - 0.5f;
        float y = (i / (rows - 1.0f)) - 0.5f;
        vertices.push_back(
          {.position = {x, y, 0.0f}, .texture_coordinate = {x + 0.5f, y + 0.5f}}
        );
      }
    }

    for (uint32_t i = 0; i < rows - 1; i++) {
      for (uint32_t j = 0; j < columns - 1; j++) {
        uint32_t a = (i * columns) + j;
        uint32_t b = a + 1;
        uint32_t c = a + columns;
        uint32_t d = c + 1;
        indices.push_back(a);
        indices.push_back(b);
        indices.push_back(c);
        indices.push_back(c);
        indices.push_back(b);
        indices.push_back(d);
      }
    }

    return {.vertices = vertices, .indices = indices};
  }
}
