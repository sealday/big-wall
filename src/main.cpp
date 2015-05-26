#include <iostream>

#include "glad/glad.h"
#include "KHR/khrplatform.h"

#include <GLFW/glfw3.h>

#include "SOIL.h"

#include "camera.h"
#include "shader_strings.h"

#define data(name) DATA#name

#include <vector>

void error_callback(int error, const char *description);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);


GLuint loadCubemap(std::vector<const GLchar*> faces);


void APIENTRY debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity,
                    GLsizei length, const GLchar *msg, const void *data);

enum VAO {
    MAIN,
    SKY,
    VAO_NUMBER
};

enum VBO {
    TRIANGLE,
    BOX,
    VBO_NUMBER
};

GLuint vaos[VAO_NUMBER], vbos[VBO_NUMBER];

const int WIDTH = 800;
const int HEIGHT = 600;
bool keys[1024];
GLFWwindow *window;
GLuint program;
GLuint skyProgram;
GLuint cubemapTexture;

GLint modelLoc, viewLoc, projLoc;

// Camera
Camera camera(glm::vec3(0.0f, 1.0f, 3.0f));
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

void do_movement();


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
    glfwWindowHint(GLFW_SAMPLES, 4);

    window = glfwCreateWindow(WIDTH, HEIGHT, "opengl", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    const GLFWvidmode *vidmode = glfwGetVideoMode(glfwGetPrimaryMonitor());

    glfwSetWindowPos(window, (vidmode->width - WIDTH)/2, (vidmode->height - HEIGHT)/2);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (!gladLoadGL()) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
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


    GLuint wallTexture, floorTexture;
    int width, height;
    unsigned char *image;
    glGenTextures(1, &wallTexture);
    glBindTexture(GL_TEXTURE_2D, wallTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    image = SOIL_load_image(WALL, &width, &height, 0, SOIL_LOAD_RGB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    SOIL_free_image_data(image);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenTextures(1, &floorTexture);
    glBindTexture(GL_TEXTURE_2D, floorTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    image = SOIL_load_image(FLOOR, &width, &height, 0, SOIL_LOAD_RGB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    SOIL_free_image_data(image);
    glBindTexture(GL_TEXTURE_2D, 0);



    while (!glfwWindowShouldClose(window)) {
        // Set frame time
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        do_movement();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view(camera.GetViewMatrix());
        glm::mat4 projection(glm::perspective(camera.Zoom, (float)WIDTH/HEIGHT, 0.1f, 1000.0f));


        glDepthMask(GL_FALSE);
        glUseProgram(skyProgram);
        glm::mat4 model;
        model = glm::scale(model, glm::vec3(20.0f, 20.0f, 20.f));
        modelLoc = glGetUniformLocation(skyProgram, "model");
        viewLoc = glGetUniformLocation(skyProgram, "view");
        projLoc = glGetUniformLocation(skyProgram, "projection");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);
        glBindVertexArray(vaos[SKY]);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthMask(GL_TRUE);


        glUseProgram(program);
        modelLoc = glGetUniformLocation(skyProgram, "model");
        viewLoc = glGetUniformLocation(program, "view");
        projLoc = glGetUniformLocation(program, "projection");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);

        glBindVertexArray(vaos[MAIN]);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, wallTexture);
        glUniform1i(glGetUniformLocation(program, "texture1"), 0);

        for (int j = 0; j < 4; j++)
            for (int i = -10; i <= 10; i++) {
                glm::mat4 model;
                model = glm::translate(model, glm::vec3(1.0f * i, 1.0f * j + 1, 0.0f));
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);

                glDrawArrays(GL_TRIANGLES, 0, 36);
            }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floorTexture);
        glUniform1i(glGetUniformLocation(program, "texture1"), 0);

        for (int i = -10; i <= 10; i++)
            for (int j = -10; j <= 10; j++)
            {
                glm::mat4 model;
                model = glm::translate(model, glm::vec3(1.0f * i, 0.0f, 1.0f * j));
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }

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
    camera.ProcessMouseScroll(yoffset);
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = ypos;
        lastY = ypos;
        firstMouse = false;
    }

    GLfloat xoffset = xpos - lastX;
    GLfloat yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void APIENTRY debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity,
                    GLsizei length, const GLchar *msg, const void *data)
{
    std::cout << msg << std::endl;
}

void do_movement()
{
    // Camera controls
    if (keys[GLFW_KEY_W])
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (keys[GLFW_KEY_S])
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (keys[GLFW_KEY_A])
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (keys[GLFW_KEY_D])
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

void init()
{
    GLuint vShader =  glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vShader, 1, &glsl::vShader, NULL);
    glCompileShader(vShader);

    GLuint  fShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fShader, 1, &glsl::fShader, NULL);
    glCompileShader(fShader);

    program = glCreateProgram();
    glAttachShader(program, vShader);
    glAttachShader(program, fShader);
    glLinkProgram(program);

    glDeleteShader(vShader);
    glDeleteShader(fShader);

    // for now just use it
    glUseProgram(program);

    GLint colorLoc = glGetUniformLocation(program, "useColor");
    glUniform3f(colorLoc, 0.9f, 0.8f, 0.2f);

    modelLoc = glGetUniformLocation(program, "model");
    viewLoc = glGetUniformLocation(program, "view");
    projLoc = glGetUniformLocation(program, "projection");

    GLfloat vertices[] = {
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
            0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

            -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

            0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
            0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
            0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };

    glGenVertexArrays(VAO_NUMBER, vaos);
    glGenBuffers(VBO_NUMBER, vbos);

    glBindVertexArray(vaos[MAIN]);

    glBindBuffer(GL_ARRAY_BUFFER, vbos[TRIANGLE]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));

    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.3f, 0.4f, 0.5f, 0.0f);

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);


    std::vector<const GLchar*> faces;
    faces.push_back(data(right.jpg));
    faces.push_back(data(left.jpg));
    faces.push_back(data(top.jpg));
    faces.push_back(data(bottom.jpg));
    faces.push_back(data(back.jpg));
    faces.push_back(data(front.jpg));
    cubemapTexture = loadCubemap(faces);

    GLfloat skyboxVertices[] = {
            // Positions
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f,  1.0f
    };

    skyProgram = glCreateProgram();
    GLuint skyVShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint skyFShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(skyVShader, 1, &glsl::skyVShader, NULL);
    glShaderSource(skyFShader, 1, &glsl::skyFShader, NULL);
    glCompileShader(skyVShader);
    glCompileShader(skyFShader);
    glAttachShader(skyProgram, skyVShader);
    glAttachShader(skyProgram, skyFShader);
    glLinkProgram(skyProgram);
    glDeleteShader(skyVShader);
    glDeleteShader(skyFShader);


    glBindVertexArray(vaos[SKY]);

    glBindBuffer(GL_ARRAY_BUFFER, vbos[BOX]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindVertexArray(0);

}

GLuint loadCubemap(std::vector<const GLchar*> faces)
{
    GLuint textureID;
    glGenTextures(1, &textureID);
    glActiveTexture(GL_TEXTURE0);

    int width,height;
    unsigned char* image;

    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
    for(GLuint i = 0; i < faces.size(); i++)
    {
        image = SOIL_load_image(faces[i], &width, &height, 0, SOIL_LOAD_RGB);
        glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
                GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image
        );
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    return textureID;
}
