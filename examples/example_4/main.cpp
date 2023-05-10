// Include necessary libraries and headers.
#include "shaders/square.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>
#include <iostream>
#include <set>
#include <cmath>

// -----------------------------------------------------------------------------
// FUNCTION PROTOTYPES
// -----------------------------------------------------------------------------

// Shader compilation.
GLuint CompileShader(const std::string& vertexShader,
    const std::string& fragmentShader);

// Create a square VAO (Vertex Array Object) with texture coordinates.
GLuint CreateSquare();

// Apply transformations to our object based on the elapsed time.
void Transform(const float , const GLuint);

// Set up the camera based on the elapsed time.
void Camera(const float , const GLuint);

// Load texture using the STB Image library and bind it to the specified texture slot.
GLuint load_opengl_texture(const std::string& filepath, GLuint slot);

// Clean up VAO to free resources.
void CleanVAO(GLuint& vao);

// Callback for OpenGL debug messages.
void GLAPIENTRY MessageCallback(GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    const void* userParam);

// -----------------------------------------------------------------------------
// ENTRY POINT
// -----------------------------------------------------------------------------

int main()
{
    // Initialize the GLFW library.
    if (!glfwInit())
    {
        std::cerr << "GLFW initialization failed." << '\n';
        std::cin.get();
        return EXIT_FAILURE;
    }

    // Configure GLFW settings.
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a GLFW window.
    auto window = glfwCreateWindow(1200, 1200, "Lab04", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cerr << "GLFW failed on window creation." << '\n';
        std::cin.get();
        glfwTerminate();
        return EXIT_FAILURE;
    }

    // Set the OpenGL context to the window we just created.
    glfwMakeContextCurrent(window);

    // Load OpenGL functions using GLAD.
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }

    // Enable OpenGL debug features.
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(MessageCallback, 0);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

    // Create a VAO for our square.
    auto squareVAO = CreateSquare();

    // Compile vertex and fragment shaders.
    GLuint squareShaderProgram = CompileShader(squareVertexShaderSrc,
        squareFragmentShaderSrc);

    // Specify the layout of the vertex data (position, color, texture coordinates).
    GLint posAttrib = glGetAttribLocation(squareShaderProgram, "aPosition");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), 0);

    GLint colorAttrib = glGetAttribLocation(squareShaderProgram, "aColor");
    glEnableVertexAttribArray(colorAttrib);
    glVertexAttribPointer(colorAttrib, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));

    // Texture coordinates for the square.
    GLint texAttrib = glGetAttribLocation(squareShaderProgram, "aTexcoord");
    glEnableVertexAttribArray(texAttrib);
    glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(5 * sizeof(GLfloat)));

    // Load and bind two textures to texture units 0 and 1.
    auto texture0 = load_opengl_texture(std::string(TEXTURES_DIR) + "/cat.png", 0);
    auto texture1 = load_opengl_texture(std::string(TEXTURES_DIR) + "/dog.png", 1);

    // Set clear color for glClear.
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    double currentTime = 0.0;
    glfwSetTime(0.0);
    while (!glfwWindowShouldClose(window))
    {
        // Process events.
        glfwPollEvents();

        // Get current time for animations.
        currentTime = glfwGetTime();

        // Clear the screen.
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw square with textures.
        glUseProgram(squareShaderProgram);
        auto samplerSlotLocation0 = glGetUniformLocation(squareShaderProgram, "uTextureA");
        auto samplerSlotLocation1 = glGetUniformLocation(squareShaderProgram, "uTextureB");
        glBindVertexArray(squareVAO);
        Transform(currentTime, squareShaderProgram);
        Camera(currentTime, squareShaderProgram);
        glUniform1i(samplerSlotLocation0, 0);  // Bind cat texture to texture unit 0
        glUniform1i(samplerSlotLocation1, 1);  // Bind dog texture to texture unit 1
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (const void*)0);

        // Swap buffers to display the rendered frame.
        glfwSwapBuffers(window);

        // Exit loop on escape key press.
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
    }

    // Clean up VAO.
    CleanVAO(squareVAO);

    // Clean up GLFW.
    glfwTerminate();
    return EXIT_SUCCESS;
}

// -----------------------------------------------------------------------------
// COMPILE SHADER
// -----------------------------------------------------------------------------
GLuint CompileShader(const std::string& vertexShaderSrc,
    const std::string& fragmentShaderSrc)
{

    auto vertexSrc = vertexShaderSrc.c_str();
    auto fragmentSrc = fragmentShaderSrc.c_str();

    auto vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSrc, nullptr);
    glCompileShader(vertexShader);

    auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSrc, nullptr);
    glCompileShader(fragmentShader);

    auto shaderProgram = glCreateProgram();

    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);

    glBindFragDataLocation(shaderProgram, 0, "outColor");
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

// -----------------------------------------------------------------------------
//  CREATE SQUARE
// -----------------------------------------------------------------------------
GLuint CreateSquare()
{

    GLfloat square[4 * 7] =
    {
      /*
    |position     |color            |texture coord*/
    -0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
    -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
     0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
     0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
    };

    GLuint square_indices[6] = { 0,1,2,0,2,3 };

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);

    GLuint ebo;
    glGenBuffers(1, &ebo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(square), square, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(square_indices), square_indices, GL_STATIC_DRAW);

    return vao;
}

GLuint load_opengl_texture(const std::string& filepath, GLuint slot)
{
    /**
     *  - Use the STB Image library to load a texture in here
     *  - Initialize the texture into an OpenGL texture
     *    - This means creating a texture with glGenTextures or glCreateTextures (4.5)
     *    - And transferring the loaded texture data into this texture
     *    - And setting the texture format
     *  - Finally return the valid texture
     */

     /** Image width, height, bit depth */
    int w, h, bpp;
    auto pixels = stbi_load(filepath.c_str(), &w, &h,&bpp, STBI_rgb_alpha);

    /*Generate a texture object and upload the loaded image to it.*/
    GLuint tex;
    glGenTextures(1, &tex);
    glActiveTexture(GL_TEXTURE0 + slot);//Texture Unit
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    /** Set parameters for the texture */
    //Wrapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //Filtering 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    /** Very important to free the memory returned by STBI, otherwise we leak */
    if(pixels)
        stbi_image_free(pixels);

    return tex;
}



// -----------------------------------------------------------------------------
// Clean VAO
// -----------------------------------------------------------------------------
void CleanVAO(GLuint& vao)
{
  glBindVertexArray(vao);
  GLint maxVertexAttribs;
  glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxVertexAttribs);
  for(GLint i = 0; i < maxVertexAttribs; i++)
    {
    glDisableVertexAttribArray(i);
    }
  glBindVertexArray(0);
  glDeleteVertexArrays(1, &vao);
  vao = 0;
}

// -----------------------------------------------------------------------------
// Code handling the transformation of objects in the Scene
// -----------------------------------------------------------------------------
void Transform(const float time, const GLuint shaderprogram)
{

    //Presentation below purely for ease of viewing individual components of calculation, and not at all necessary.

    //Translation moves our object.        base matrix      Vector for movement along each axis
    glm::mat4 translation = glm::translate(glm::mat4(1), glm::vec3(sin(time), 0.f, 0.f));

    //Rotate the object                    base matrix      degrees to rotate          axis to rotate around
    glm::mat4 rotation = glm::rotate(glm::mat4(1), glm::radians(time) * 10, glm::vec3(0, 0, 1));

    //Scale the object                     base matrix      vector containing how much to scale along each axis (here the same for all axis)
    glm::mat4 scale = glm::scale(glm::mat4(1), glm::vec3(sin(time)));

    //Create transformation matrix      These must be multiplied in this order, or the results will be incorrect
    glm::mat4 transformation = translation * rotation * scale;


    //Get uniform to place transformation matrix in
    //Must be called after calling glUseProgram         shader program in use   Name of Uniform
    GLuint transformationmat = glGetUniformLocation(shaderprogram, "u_TransformationMat");

    //Send data from matrices to uniform
    //                 Location of uniform  How many matrices we are sending    value_ptr to our transformation matrix
    glUniformMatrix4fv(transformationmat, 1, false, glm::value_ptr(transformation));
}

// -----------------------------------------------------------------------------
// Code handling the camera
// -----------------------------------------------------------------------------
void Camera(const float time, const GLuint shaderprogram)
{

    //Matrix which helps project our 3D objects onto a 2D image. Not as relevant in 2D projects
    //The numbers here represent the aspect ratio. Since our window is a square, aspect ratio here is 1:1, but this can be changed.
    glm::mat4 projection = glm::ortho(-1.f, 1.f, -1.f, 1.f);

    //Matrix which defines where in the scene our camera is
    //                           Position of camera     Direction camera is looking     Vector pointing upwards
    glm::mat4 view = glm::lookAt(glm::vec3(0, 0, -1), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

    //Get unforms to place our matrices into
    GLuint projmat = glGetUniformLocation(shaderprogram, "u_ProjectionMat");
    GLuint viewmat = glGetUniformLocation(shaderprogram, "u_ViewMat");

    //Send data from matrices to uniform
    glUniformMatrix4fv(projmat, 1, false, glm::value_ptr(projection));
    glUniformMatrix4fv(viewmat, 1, false, glm::value_ptr(view));
}

// -----------------------------------------------------------------------------
// MessageCallback (for debugging purposes)
// -----------------------------------------------------------------------------
void GLAPIENTRY
MessageCallback(GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    const void* userParam)
{
    std::cerr << "GL CALLBACK:" << (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "") <<
        "type = 0x" << type <<
        ", severity = 0x" << severity <<
        ", message =" << message << "\n";
}
