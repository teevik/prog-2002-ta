#include "shaders/square.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <set>
#include <cmath>

// -----------------------------------------------------------------------------
// FUNCTION PROTOTYPES
// -----------------------------------------------------------------------------
GLuint CompileShader(const std::string& vertexShader,
                     const std::string& fragmentShader);

GLuint CreateSquare();

void Transform(const float, const GLuint);

void Camera(const float, const GLuint);

void CleanVAO(GLuint &vao);

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
    // Initialization of GLFW
    if(!glfwInit())
    {
        std::cerr << "GLFW initialization failed." << '\n';
        std::cin.get();

        return EXIT_FAILURE;
    }

    // Setting window hints
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    auto window = glfwCreateWindow(1200, 1200, "Lab02", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cerr << "GLFW failed on window creation." << '\n';
        std::cin.get();

        glfwTerminate();

        return EXIT_FAILURE;
    }

    // Setting the OpenGL context.
    glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
    std::cout << "Failed to initialize GLAD" << std::endl;
    glfwTerminate();
    return EXIT_FAILURE;
    }

    // Eanable capture of debug output.
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(MessageCallback, 0);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

    auto squareVAO = CreateSquare();
    auto squareShaderProgram = CompileShader(squareVertexShaderSrc,
                                             squareFragmentShaderSrc);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    double currentTime = 0.0;
    glfwSetTime(0.0);
    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Time management
        currentTime = glfwGetTime();

        glClear(GL_COLOR_BUFFER_BIT);

        // Draw SQUARE
        auto vertexColorLocation = glGetUniformLocation(squareShaderProgram, "u_Color");
        glUseProgram(squareShaderProgram);
        glBindVertexArray(squareVAO);
        glUniform4f(vertexColorLocation, 0.0f, 1.f, 0.0f, 1.0f);
        Transform(currentTime, squareShaderProgram);
        Camera(currentTime, squareShaderProgram);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (const void*)0);

        glfwSwapBuffers(window);

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            break;
        }
    }

    glUseProgram(0);
    glDeleteProgram(squareShaderProgram);

    CleanVAO(squareVAO);

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

    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

// -----------------------------------------------------------------------------
//  CREATE SQUARE
// -----------------------------------------------------------------------------
GLuint CreateSquare()
{

    GLfloat square[4*3] =
        {
        -0.5f, 0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        0.5f, 0.5f, 0.0f,
    };

    GLuint square_indices[6] = {0,1,2,0,2,3};

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);

    GLuint ebo;
    glGenBuffers(1, &ebo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(square), square, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*3, (const void *)0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(square_indices), square_indices, GL_STATIC_DRAW);

    return vao;
}

// -----------------------------------------------------------------------------
// Clean VAO
// -----------------------------------------------------------------------------
void CleanVAO(GLuint &vao)
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
    glm::mat4 translation= glm::translate( glm::mat4(1),    glm::vec3(sin(time),0.f,0.f));

    //Rotate the object                    base matrix      degrees to rotate          axis to rotate around
    glm::mat4 rotation   = glm::rotate(    glm::mat4(1),    glm::radians(time)*10,     glm::vec3(0,0,1));

    //Scale the object                     base matrix      vector containing how much to scale along each axis (here the same for all axis)
    glm::mat4 scale      = glm::scale(     glm::mat4(1),    glm::vec3(sin(time)));

    //Create transformation matrix      These must be multiplied in this order, or the results will be incorrect
    glm::mat4 transformation =          translation * rotation * scale;


    //Get uniform to place transformation matrix in
    //Must be called after calling glUseProgram         shader program in use   Name of Uniform
    GLuint transformationmat = glGetUniformLocation(    shaderprogram,          "u_TransformationMat");

    //Send data from matrices to uniform
    //                 Location of uniform  How many matrices we are sending    value_ptr to our transformation matrix
    glUniformMatrix4fv(transformationmat,   1, false,                           glm::value_ptr(transformation));
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
    glm::mat4 view = glm::lookAt(glm::vec3(0, 0, -1),   glm::vec3(0, 0, 0),             glm::vec3(0, 1, 0));

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
MessageCallback( GLenum source,
                 GLenum type,
                 GLuint id,
                 GLenum severity,
                 GLsizei length,
                 const GLchar* message,
                 const void* userParam )
{
    std::cerr << "GL CALLBACK:" << ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ) <<
        "type = 0x" << type <<
        ", severity = 0x" << severity <<
        ", message =" << message << "\n";
}
