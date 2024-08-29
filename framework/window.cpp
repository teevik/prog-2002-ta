#include "window.h"
#include <ios>
#include <iostream>

using namespace framework;

Window::Window(
  int32_t width, int32_t height, const string &title, bool resizable
) {
  auto error_callback = [](int code, const char *description) {
    std::cerr << "GLFW Error (0x" << std::hex << code << "): " << description
              << "\n";
  };
  glfwSetErrorCallback(error_callback);

  if (!glfwInit()) {
    throw std::runtime_error("Failed to initialize GLFW.");
  }

  glfwWindowHint(GLFW_RESIZABLE, resizable);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  glfw_window =
    glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

  if (glfw_window == nullptr) {
    glfwTerminate();
    throw std::runtime_error("Failed to create GLFW window.");
  }

  glfwMakeContextCurrent(glfw_window);

  auto glew_error = glewInit();
  if (glew_error != GLEW_OK) {
    auto error_message =
      reinterpret_cast<const char *>(glewGetErrorString(glew_error));

    glfwTerminate();
    throw std::runtime_error(
      std::string("Failed to initialize GLEW: ") + error_message
    );
  }

  auto message_callback = [](
                            GLenum source,
                            GLenum type,
                            GLuint id,
                            GLenum severity,
                            GLsizei length,
                            const GLchar *message,
                            const void *userParam
                          ) {
    std::cerr << "OpenGL Callback: "
              << (type == GL_DEBUG_TYPE_ERROR ? "** ERROR **" : "")
              << " Type: 0x" << type << ", Severity: 0x" << severity
              << ", Message: " << message << "\n";
  };

  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(message_callback, 0);

  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &default_framebuffer);

  std::cout << "Vendor: " << glGetString(GL_VENDOR) << "\n";
  std::cout << "Renderer: " << glGetString(GL_RENDERER) << "\n";
  std::cout << "OpenGL version: " << glGetString(GL_VERSION) << "\n";
}

Window::Window(Window &&window) noexcept : glfw_window(window.glfw_window) {
  window.glfw_window = nullptr;
}

Window::~Window() {
  if (glfw_window) {
    glfwDestroyWindow(glfw_window);
    glfwTerminate();
  }
}

bool Window::should_close() const {
  return glfwWindowShouldClose(glfw_window);
}

bool Window::get_key(int key) const {
  return glfwGetKey(glfw_window, key);
}

void Window::clear(Clear to_clear) const {
  uint32_t clear_bits = 0;

  if (to_clear.color.has_value()) {
    auto color = to_clear.color.value();
    glClearColor(color[0], color[1], color[2], color[3]);
    clear_bits |= GL_COLOR_BUFFER_BIT;
  }

  if (to_clear.depth.has_value()) {
    auto depth = to_clear.depth.value();
    glClearDepth(depth);
    clear_bits |= GL_DEPTH_BUFFER_BIT;
  }

  if (to_clear.stencil.has_value()) {
    auto stencil = to_clear.stencil.value();
    glClearStencil(stencil);
    clear_bits |= GL_STENCIL_BUFFER_BIT;
  }

  if (clear_bits != 0) {
    glClear(clear_bits);
  }
}

void Window::begin_default_pass(optional<Clear> pass_action) const {
  int width, height;
  glfwGetWindowSize(glfw_window, &width, &height);

  glBindFramebuffer(GL_FRAMEBUFFER, default_framebuffer);
  glViewport(0, 0, width, height);
  glScissor(0, 0, width, height);

  if (pass_action.has_value()) {
    auto to_clear = pass_action.value();
    clear(to_clear);
  }
}

void Window::commit_frame() const {
  glfwSwapBuffers(glfw_window);
  glfwPollEvents();
}
