/* =======================================================================================
    Copyright 2023 NTNU

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
=======================================================================================*/


/* === HEADER INCLUSIONS ===
 * We begin by including essential C++ libraries and the required headers for GLAD and GLFW.
 * - <cstdlib> gives us access to general purpose functions and macros.
 * - <ios> and <iostream> are used for input-output stream operations.
 * - <glad/glad.h> and <GLFW/glfw3.h> are the header files necessary for using GLAD and GLFW functionalities respectively.
 */

// External libs includes
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Standard libs includes
#include <cstdlib>
#include <ios>
#include <iostream>

// Function Prototypes
void GLFWErrorCallback(int code, const char* description);
void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
                                GLsizei length, const GLchar* message, const void* userParam);

/* === FUNCTION PROTOTYPES ===
 * We declare prototypes for our callback functions upfront so the main function can reference them.
 * These functions will be defined later and are used for handling errors and debug messages.
 */

int main(void)
{
    /* === GLFW SETUP & INITIALIZATION ===
     * GLFW is a library for handling window creation, input, etc. for OpenGL.
     * We set a custom error callback for GLFW to give more meaningful error messages.
     * If GLFW fails to initialize, it's crucial to halt the program and let the user know.
     */
    glfwSetErrorCallback(GLFWErrorCallback);
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW." << std::endl;
        std::cin.get();
        return EXIT_FAILURE;
    }

    /* === CONFIGURING GLFW WINDOW PROPERTIES ===
     * We're setting specific properties for our window. These include:
     * - Disabling resizing
     * - Specifying the desired OpenGL version (4.3 in this case)
     * - Using the core profile of OpenGL.
     */
    glfwWindowHint(GLFW_RESIZABLE, false);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /* === WINDOW CREATION ===
     * A window of size 800x600 pixels is created.
     * If the window fails to create, GLFW is terminated and the program exits.
     */
    auto window = glfwCreateWindow(800, 600, "Hello Triangle", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cerr << "Failed to create GLFW window." << std::endl;
        glfwTerminate();
        std::cin.get();
        return EXIT_FAILURE;
    }

    /* === SETTING CURRENT CONTEXT ===
     * OpenGL operations require a context. The context represents the "environment"
     * where the OpenGL resources (like textures, shaders) live. Here, we set the context
     * of our just created window as the current one.
     */
    glfwMakeContextCurrent(window);

    /* === GLAD INITIALIZATION ===
     * GLAD is a function loader for OpenGL. We need it to access modern OpenGL functions.
     * If it fails to initialize, we again exit gracefully.
     */
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD." << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }

    /* === SETTING OPENGL DEBUG OUTPUT ===
     * Debug output is valuable during development to catch errors and understand behavior.
     * We set a callback to print messages from OpenGL.
     */
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(MessageCallback, 0);

    /* === PRINTING OPENGL INFORMATION ===
     * It's often useful to log the vendor, renderer, and version info.
     * This can help debug issues on different hardware or drivers.
     */
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << "\n";
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << "\n";
    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << "\n";

    /* === OPENGL SETUP ===
     * This section sets the clear color for the window's background
     * and defines a simple triangle's vertices.
     */
    glClearColor(0.5f, 0.0f, 0.0f, 1.0f);
    GLfloat triangle[3*2] = {
        -0.5f, -0.5f,
        0.5f, -0.5f,
        0.0f, 0.5f
    };

    /* === BUFFER SETUP FOR TRIANGLE ===
     * OpenGL requires buffers to hold vertex data.
     * We create a Vertex Array Object (VAO) and a Vertex Buffer Object (VBO),
     * bind them, and populate the VBO with our triangle's vertices.
     */
    GLuint vertexArrayId;
    glGenVertexArrays(1, &vertexArrayId);
    glBindVertexArray(vertexArrayId);

    GLuint vertexBufferId;
    glGenBuffers(1, &vertexBufferId);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangle), triangle, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float)*2, nullptr);
    glEnableVertexAttribArray(0);

    /* === SHADER SETUP ===
     * Shaders are programs that run on the GPU.
     * We define, compile, and link a basic vertex and fragment shader.
     * The vertex shader processes each vertex and the fragment shader processes each pixel.
     */
    const std::string vertexShaderSrc = R"(
#version 430 core
layout(location = 0) in vec4 position;
void main()
{
    gl_Position = position;
}
)";

    const std::string fragmentShaderSrc = R"(
#version 430 core
out vec4 color;
void main()
{
    color = vec4(1);
}
)";

    auto vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const GLchar* vss = vertexShaderSrc.c_str();
    glShaderSource(vertexShader, 1, &vss, nullptr);
    glCompileShader(vertexShader);

    auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar* fss = fragmentShaderSrc.c_str();
    glShaderSource(fragmentShader, 1, &fss, nullptr);
    glCompileShader(fragmentShader);

    auto shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glUseProgram(shaderProgram);

    /* === MAIN RENDER LOOP ===
     * Until the window is closed or the ESC key is pressed, we:
     * - Poll for events (like input).
     * - Clear the screen.
     * - Draw our triangle.
     * - Swap buffers to show our triangle.
     */
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glfwSwapBuffers(window);
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) break;
    }

    /* === CLEANUP ===
     * Freeing resources after they are no longer needed is good practice.
     * We delete our buffers and terminate GLFW.
     */
    glDisableVertexAttribArray(0);
    glDeleteBuffers(1, &vertexBufferId);
    glDeleteVertexArrays(1, &vertexArrayId);

    glfwTerminate();

    return EXIT_SUCCESS;
}

// Function Definitions

void GLFWErrorCallback(int code, const char* description)
{
    std::cerr << "GLFW Error (0x" << std::hex << code << "): " << description << "\n";
}

void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
                                GLsizei length, const GLchar* message, const void* userParam)
{
    std::cerr << "OpenGL Callback: " << (type == GL_DEBUG_TYPE_ERROR ? "** ERROR **" : "")
              << " Type: 0x" << type
              << ", Severity: 0x" << severity
              << ", Message: " << message << "\n";
}
