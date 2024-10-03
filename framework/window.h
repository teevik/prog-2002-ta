#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <array>
#include <cstdint>
#include <optional>
#include <string>

using std::array;
using std::optional;
using std::string;

namespace framework {
  struct Clear {
    optional<array<float, 4>> color = array{0.0f, 0.0f, 0.0f, 0.0f};
    optional<float> depth = 1.;
    optional<int32_t> stencil = std::nullopt;
  };

  struct Window {
    GLFWwindow *glfw_window = nullptr;
    int32_t default_framebuffer;

    Window(
      int32_t width, int32_t height, const string &title, bool resizable = true
    );

    Window(Window &&window) noexcept;

    ~Window();

    bool should_close() const;

    bool get_key(int key) const;

    float time() const;

    float get_aspect_ratio() const;

    void clear(Clear to_clear) const;

    void begin_default_pass(optional<Clear> pass_action = optional(Clear{}))
      const;

    void commit_frame() const;
  };
}
