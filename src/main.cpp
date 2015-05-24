#include <iostream>

#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include "camera.h"
#include "shader_strings.h"


void error_callback(int error, const char *description);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);

enum VAO {
    MAIN,
    VAO_NUMBER
};

enum VBO {
    TRIANGLE,
    VBO_NUMBER
};

GLuint vaos[VAO_NUMBER], vbos[VBO_NUMBER];



void init();

int main() {

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        std::cout << "can't not init glfw" << std::endl;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // for mac
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);


    GLFWwindow *window = glfwCreateWindow(800, 600, "opengl", NULL, NULL);
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

    init();

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

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
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{

}

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{

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

    // for now just use it
    glUseProgram(program);

    GLint colorLoc = glGetUniformLocation(program, "useColor");
    glUniform3f(colorLoc, 0.9f, 0.8f, 0.2f);

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


    glClearColor(0.3f, 0.4f, 0.5f, 1.0f);
}
