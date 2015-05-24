#include <iostream>

#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include "camera.h"
#include "shader_strings.h"


void error_callback(int error, const char *description);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity,
                    GLsizei length, const GLchar *msg, void *data);

enum VAO {
    MAIN,
    VAO_NUMBER
};

enum VBO {
    TRIANGLE,
    VBO_NUMBER
};

GLuint vaos[VAO_NUMBER], vbos[VBO_NUMBER];

const int WIDTH = 800;
const int HEIGHT = 600;
bool keys[1024];
GLFWwindow *window;

GLint modelLoc, viewLoc, projLoc;

void init();


int main() {

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        std::cout << "can't not init glfw" << std::endl;
    }
#ifndef __APPLE__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
#endif
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // for mac
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "opengl", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {

        std::cerr << "Failed to initialize GLEW" << std::endl;
        exit(EXIT_FAILURE);
    }


    // some callback
    glfwSetKeyCallback(window, key_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
#ifndef __APPLE__
    glDebugMessageCallback(debug_callback, 0);
#endif

    init();

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindVertexArray(vaos[MAIN]);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

void error_callback(int error, const char *description)
{
    std::cerr << description << std::endl;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (action == GLFW_PRESS)
        keys[key] = true;
    else if (action == GLFW_RELEASE)
        keys[key] = false;
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{

}

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{

}

void debug_callback(GLenum source, GLenum type, GLuint id,GLenum severity,
                    GLsizei length, const GLchar *msg, void *data )
{
    std::cout << msg << std::endl;
}

void init()
{
    GLint success;
    GLchar infoLog[512];
    GLuint vShader =  glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vShader, 1, &glsl::vShader, NULL);
    glCompileShader(vShader);
    glGetShaderiv(vShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vShader, 512, NULL, infoLog);
        std::cerr << "compile vertex shader failure" << std::endl;
        std::cerr << infoLog << std::endl;
    }

    GLuint  fShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fShader, 1, &glsl::fShader, NULL);
    glCompileShader(fShader);
    glGetShaderiv(fShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fShader, 512, NULL, infoLog);
        std::cerr << "compile fragment shader failure" << std::endl;
        std::cerr << infoLog << std::endl;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vShader);
    glAttachShader(program, fShader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cerr << "link program failure" << std::endl;
        std::cerr << infoLog << std::endl;
    }
    glDeleteShader(vShader);
    glDeleteShader(fShader);

    // for now just use it
    glUseProgram(program);

    GLint colorLoc = glGetUniformLocation(program, "useColor");
    glUniform3f(colorLoc, 0.9f, 0.8f, 0.2f);

    modelLoc = glGetUniformLocation(program, "model");
    viewLoc = glGetUniformLocation(program, "view");
    projLoc = glGetUniformLocation(program, "projection");



    glm::mat4 mat4;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &mat4[0][0]);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &mat4[0][0]);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, &mat4[0][0]);

    GLfloat vertices[] = {
             0.0f,  0.5f,  0.0f,
            -0.5f, -0.5f,  0.0f,
             0.5f, -0.5f,  0.0f
    };

    glGenVertexArrays(VAO_NUMBER, vaos);
    glGenBuffers(VBO_NUMBER, vbos);

    glBindVertexArray(vaos[MAIN]);

    glBindBuffer(GL_ARRAY_BUFFER, vbos[TRIANGLE]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.3f, 0.4f, 0.5f, 0.0f);

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);


}
