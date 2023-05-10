#include "shaders/shader.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "tiny_obj_loader.h"

#include <iostream>
#include <set>
#include <cmath>

//------------------------------------------------------------------------------
// VERTEX STRUCT
//------------------------------------------------------------------------------
struct Vertex
{
    glm::vec3 location;
    glm::vec3 normals;
    glm::vec2 texCoords;
};


// -----------------------------------------------------------------------------
// FUNCTION PROTOTYPES
// -----------------------------------------------------------------------------
GLuint CompileShader(const std::string& vertexShader,
    const std::string& fragmentShader);

GLuint CreateSquare();

GLuint LoadModel(const std::string, int& size);

void Camera(const float, const GLuint);

void Transform(const float, const GLuint);

void Light(const float, const GLuint);

void CleanVAO(GLuint& vao);

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
    if (!glfwInit())
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

    auto window = glfwCreateWindow(1200, 1200, "Lab05", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cerr << "GLFW failed on window creation." << '\n';
        std::cin.get();

        glfwTerminate();

        return EXIT_FAILURE;
    }

    // Setting the OpenGL context.
    glfwMakeContextCurrent(window);

    	//Load GLAD so it configures OpenGL
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		glfwTerminate();
		return EXIT_FAILURE;
	}

    // Eanable capture of debug output.
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(MessageCallback, 0);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

    glEnable(GL_DEPTH_TEST);


    int size = 0;
    auto potVAO = LoadModel(std::string(MODELS_DIR), size);
    auto ShaderProgram = CompileShader(VertexShaderSrc, directionalLightFragmentShaderSrc);
    ///auto ShaderProgram = CompileShader(VertexShaderSrc, pointLightFragmentShaderSrc); //Feel free to test with pointlights as well by un-commenting this.
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    double currentTime = 0.0;
    glfwSetTime(0.0);
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Time management
        currentTime = glfwGetTime();

        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        // Draw SQUARE
        auto vertexColorLocation = glGetUniformLocation(ShaderProgram, "u_Color");
        glUseProgram(ShaderProgram);
        glBindVertexArray(potVAO);
        glUniform4f(vertexColorLocation, 0.4f, 0.4f, 0.45f, 1.0f);
        Camera(currentTime, ShaderProgram);
        Transform(currentTime, ShaderProgram);
        Light(currentTime, ShaderProgram);
        glDrawArrays(GL_TRIANGLES, 0, size);

        glfwSwapBuffers(window);

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            break;
        }
    }

    glUseProgram(0);
    glDeleteProgram(ShaderProgram);

    CleanVAO(potVAO);

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
    GLint isCompiled = 0;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isCompiled);
    if (!isCompiled)
    {
        GLint maxLength = 0;
        glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);

        // The maxLength includes the NULL character
        std::vector<GLchar> errorLog(maxLength);
        glGetShaderInfoLog(vertexShader, maxLength, &maxLength, &errorLog[0]);

        // Provide the infolog in whatever manor you deem best.
        std::cout << errorLog.data() << std::endl;
        std::cin.get();
    }
    auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSrc, nullptr);
    glCompileShader(fragmentShader);

    //I spent like 3 hours trying to find shader errors. Here you go. Now this will print any error you get trying to compile a shader.
    //Don't suffer like I did.

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isCompiled);
    if (!isCompiled)
    {
        GLint maxLength = 0;
        glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);

        // The maxLength includes the NULL character
        std::vector<GLchar> errorLog(maxLength);
        glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, &errorLog[0]);

        // Provide the infolog in whatever manor you deem best.
        std::cout << errorLog.data() << std::endl;
        std::cin.get();
    }

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

    GLfloat square[4 * 3] =
    {
    -0.5f, 0.5f, 0.0f,
    -0.5f, -0.5f, 0.0f,
    0.5f, -0.5f, 0.0f,
    0.5f, 0.5f, 0.0f,
    };

    GLuint square_indices[6] = { 0,1,2,0,2,3 };

    GLuint vao;
    glCreateVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);

    GLuint ebo;
    glGenBuffers(1, &ebo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(square), square, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 3, (const void*)0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(square_indices), square_indices, GL_STATIC_DRAW);

    return vao;
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
// Code handling the camera
// -----------------------------------------------------------------------------
GLuint LoadModel(const std::string path, int& size)
{

    //We create a vector of Vertex structs. OpenGL can understand these, and so will accept them as input.
    std::vector<Vertex> vertices;

    //Some variables that we are going to use to store data from tinyObj
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials; //This one goes unused for now, seeing as we don't need materials for this model.
    
    //Some variables incase there is something wrong with our obj file
    std::string warn;
    std::string err;

    //We use tinobj to load our models. Feel free to find other .obj files and see if you can load them.
    tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, (path + "/teacup.obj").c_str(), path.c_str());

    if (!warn.empty()) {
        std::cout << warn << std::endl;
    }

    if (!err.empty()) {
        std::cerr << err << std::endl;
    }

    //For each shape defined in the obj file
    for (auto shape : shapes)
    {
        //We find each mesh
        for (auto meshIndex : shape.mesh.indices)
        {
            //And store the data for each vertice, including normals
            glm::vec3 vertice = {
                attrib.vertices[meshIndex.vertex_index * 3],
                attrib.vertices[(meshIndex.vertex_index * 3) + 1],
                attrib.vertices[(meshIndex.vertex_index * 3) + 2]
            };
            glm::vec3 normal = {
                attrib.normals[meshIndex.normal_index * 3],
                attrib.normals[(meshIndex.normal_index * 3) + 1],
                attrib.normals[(meshIndex.normal_index * 3) + 2]
            };
            glm::vec2 textureCoordinate = {                         //These go unnused, but if you want textures, you will need them.
                attrib.texcoords[meshIndex.texcoord_index * 2],
                attrib.texcoords[(meshIndex.texcoord_index * 2) + 1]
            };

            vertices.push_back({ vertice, normal, textureCoordinate }); //We add our new vertice struct to our vector

        }
    }

    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    //As you can see, OpenGL will accept a vector of structs as a valid input here
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, nullptr);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 3));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 6));
    
    //This will be needed later to specify how much we need to draw. Look at the main loop to find this variable again.
    size = vertices.size();

    return VAO;
}

void Transform(const float time, const GLuint shaderprogram)
{

    //Presentation below purely for ease of viewing individual components of calculation, and not at all necessary.

    //Translation moves our object.        base matrix      Vector for movement along each axis
    glm::mat4 translation = glm::translate(glm::mat4(1),    glm::vec3(0.f, -0.8f, 1.f));

    //Rotate the object                    base matrix      degrees to rotate          axis to rotate around
    glm::mat4 rotation = glm::rotate(glm::mat4(1),          glm::radians(time*25.f),           glm::vec3(0, 1, 0));

    //Scale the object                     base matrix      vector containing how much to scale along each axis (here the same for all axis)
    glm::mat4 scale = glm::scale(          glm::mat4(1),    glm::vec3(1.f));

    //Create transformation matrix      These must be multiplied in this order, or the results will be incorrect
    glm::mat4 transformation = translation*rotation*scale;


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
    glm::mat4 projection = glm::perspective(90.f,1.f,0.1f,60.f);

    //Matrix which defines where in the scene our camera is
    //                           Position of camera     Direction camera is looking     Vector pointing upwards
    glm::mat4 view = glm::lookAt(glm::vec3(0.f, 0.f, -1.f), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

    //Get unforms to place our matrices into
    GLuint projmat = glGetUniformLocation(shaderprogram, "u_ProjectionMat");
    GLuint viewmat = glGetUniformLocation(shaderprogram, "u_ViewMat");

    //Send data from matrices to uniform
    glUniformMatrix4fv(projmat, 1, false, glm::value_ptr(projection));
    glUniformMatrix4fv(viewmat, 1, false, glm::value_ptr(view));
}

void Light(const float time, const GLuint shaderprogram)
{
    //Get uniforms for our Light-variables.
    GLuint lightPos = glGetUniformLocation(shaderprogram, "u_LightPosition");
    GLuint lightColor = glGetUniformLocation(shaderprogram, "u_LightColor");
    GLuint lightDir = glGetUniformLocation(shaderprogram, "u_LightDirection");
    GLuint specularity = glGetUniformLocation(shaderprogram, "u_Specularity");

    //Send Variables to our shader
    glUniform3f(lightPos, cos(time), 0.0f, 1+sin(time));    //Position of a point in space. For Point lights.
    glUniform3f(lightDir, -1.f, -1.f, -1.f);                //Direction vector. For Directional Lights.

    glUniform3f(lightColor, 1.f, 1.f, 1.f);                 //RGB values
    glUniform1f(specularity, 1.f);                          //How much specular reflection we have for our object

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
