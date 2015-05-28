#include <iostream>
#include <fstream>

#include "glad/glad.h"
#include "KHR/khrplatform.h"

#include <GLFW/glfw3.h>

#include "SOIL.h"

#include "camera.h"
#include "shader_strings.h"
#include "MD2.h"

#define resource(name) DATA#name

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
    MAP,
    OBJ,
    VAO_NUMBER
};

enum VBO {
    TRIANGLE,
    BOX,
    DOT,
    OBJ_VBO,
    OBJ_UV_VBO,
    VBO_NUMBER
};

GLuint vaos[VAO_NUMBER], vbos[VBO_NUMBER];


std::vector<Frame> frames;
std::vector<std::vector<TriangleVertex>> triangleVertices;
std::vector<MeshUV> uvs;
std::vector<Vertex> vertices;
std::vector<std::vector<Vertex>> frameVertices;
std::vector<VertexUV> sts;

const int WIDTH = 800;
const int HEIGHT = 600;
bool keys[1024];
GLFWwindow *window;
GLuint program;
GLuint skyProgram;
GLuint mapProgram;
GLuint cubemapTexture;

GLint modelLoc, viewLoc, projLoc;

bool thirdPerson = false;
bool isStand = true;

// Camera
Camera camera(glm::vec3(0.0f, 1.0f, 10.0f));
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

void do_movement();


void init();


std::vector<Dot> dots;

int main() {
    dots.push_back({3, 3});
    dots.push_back({2, 2});
    dots.push_back({4, 2});
    dots.push_back({5, 2});
    dots.push_back({-10, 10});
    dots.push_back({10, 10});
    dots.push_back({10, -10});
    dots.push_back({-10, -10});
    camera.setDots(dots);
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

    // model load code ...
    std::ifstream md2File(resource(tris.md2), std::ios_base::binary);
    if (!md2File) {
        std::cerr << "load model failure" << std::endl;
        exit(-1);
    }

    MD2 md2;
    md2File.read(reinterpret_cast<char *>(&md2), sizeof(md2));
    std::string magic(reinterpret_cast<char *>(&md2.magic), 4);
    if (magic != "IDP2") {
        std::cerr << "not MD2 model" << std::endl;
        exit(-1);
    }

    md2File.seekg(md2.offsetFrames, md2File.beg);
    for (int i = 0; i < md2.numFrames; i++) {
        Frame frame;
        md2File.read(reinterpret_cast<char *>(&frame), sizeof(frame));
        frames.push_back(frame);

        std::vector<TriangleVertex> vertices;
        vertices.push_back(frame.vertices[0]);
        for (int i = 1; i < md2.numVertices; i++) {
            TriangleVertex triangleVertex;
            md2File.read(reinterpret_cast<char *>(&triangleVertex), sizeof(triangleVertex));
            vertices.push_back(triangleVertex);
        }
        triangleVertices.push_back(vertices);
    }

    md2File.seekg(md2.offsetTexCoords, md2File.beg);
    for (size_t i = 0; i < md2.numTexCoords; i++) {
        MeshUV meshUV;
        md2File.read(reinterpret_cast<char *>(&meshUV), sizeof(meshUV));
        uvs.push_back(meshUV);
    }

    for (size_t p = 0; p < frames.size(); p++) {
        for (size_t i = 0; i < triangleVertices[p].size(); i++) {
            Vertex vertex;
            vertex.coords[0] = triangleVertices[p][i].vertex[0] * frames[p].scale[0] +
                                   frames[p].translate[0];
            vertex.coords[1] = triangleVertices[p][i].vertex[2] * frames[p].scale[2] +
                               frames[p].translate[2];
            vertex.coords[2] = triangleVertices[p][i].vertex[1] * frames[p].scale[1] +
                               frames[p].translate[1];
            vertices.push_back(vertex);
        }


        std::vector<Vertex> frameVertex;
        md2File.seekg(md2.offsetTriangles, md2File.beg);
        for (size_t i = 0; i < md2.numTriangles; i++) {
            Mesh mesh;
            md2File.read(reinterpret_cast<char *>(&mesh), sizeof(mesh));
            for (size_t k = 0; k < 3; k++) {
                frameVertex.push_back(vertices[mesh.meshIndex[k]]);

                VertexUV uv;
                uv.st[0] = uvs[mesh.stIndex[k]].s / (float) md2.skinWidth;
                uv.st[1] = uvs[mesh.stIndex[k]].t / (float) md2.skinWidth;
                sts.push_back(uv);
            }
        }
        vertices.clear();
        frameVertices.push_back(frameVertex);
    }

    for (size_t i = 0; i < frames.size(); i++) {
        std::cout << i << " " << frames[i].name << std::endl;
    }

    std::cout << frameVertices[0][100].coords[0] << std::endl;
    std::cout << frameVertices[45][100].coords[0] << std::endl;
    // end model load
    // some callback
    glfwSetKeyCallback(window, key_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
#ifndef __APPLE__
    glDebugMessageCallback(debug_callback, 0);
#endif

    init();

    int texWidth, texHeight;
    unsigned char *texture;
    GLuint texture_obj;

    FILE* texFile = fopen(resource(red.pcx),"rb");

    if (texFile) {
        int imgWidth, imgHeight, texFileLen, imgBufferPtr, i;
        pcxHeader *pcxPtr;
        unsigned char *imgBuffer, *texBuffer, *pcxBufferPtr, *paletteBuffer;

        /* find length of file */
        fseek(texFile, 0, SEEK_END);
        texFileLen = ftell(texFile);
        fseek(texFile, 0, SEEK_SET);

        /* read in file */
        texBuffer = (unsigned char *) malloc(texFileLen + 1);
        fread(texBuffer, sizeof(char), texFileLen, texFile);

        /* get the image dimensions */
        pcxPtr = (pcxHeader *) texBuffer;
        imgWidth = pcxPtr->size[0] - pcxPtr->offset[0] + 1;
        imgHeight = pcxPtr->size[1] - pcxPtr->offset[1] + 1;

        /* image starts at 128 from the beginning of the buffer */
        imgBuffer = (unsigned char *) malloc(imgWidth * imgHeight);
        imgBufferPtr = 0;
        pcxBufferPtr = &texBuffer[128];
        /* decode the pcx image */
        while (imgBufferPtr < (imgWidth * imgHeight)) {
            if (*pcxBufferPtr > 0xbf) {
                int repeat = *pcxBufferPtr++ & 0x3f;
                for (i = 0; i < repeat; i++)
                    imgBuffer[imgBufferPtr++] = *pcxBufferPtr;
            } else {
                imgBuffer[imgBufferPtr++] = *pcxBufferPtr;
            }
            pcxBufferPtr++;
        }
        /* read in the image palette */
        paletteBuffer = (unsigned char *) malloc(768);
        for (i = 0; i < 768; i++)
            paletteBuffer[i] = texBuffer[texFileLen - 768 + i];

        /* find the nearest greater power of 2 for each dimension */
        {
            int imageWidth = imgWidth, imageHeight = imgHeight;
            i = 0;
            while (imageWidth) {
                imageWidth /= 2;
                i++;
            }
            texWidth = pow(2, (double) i);
            i = 0;
            while (imageHeight) {
                imageHeight /= 2;
                i++;
            }
            texHeight = pow(2, (double) i);
        }
        /* now create the OpenGL texture */
        {
            int i, j;
            texture = (unsigned char *) malloc(texWidth * texHeight * 3);
            for (j = 0; j < imgHeight; j++) {
                for (i = 0; i < imgWidth; i++) {
                    texture[3 * (j * texWidth + i) + 0]
                            = paletteBuffer[3 * imgBuffer[j * imgWidth + i] + 0];
                    texture[3 * (j * texWidth + i) + 1]
                            = paletteBuffer[3 * imgBuffer[j * imgWidth + i] + 1];
                    texture[3 * (j * texWidth + i) + 2]
                            = paletteBuffer[3 * imgBuffer[j * imgWidth + i] + 2];
                }
            }
        }
        free(paletteBuffer);
        free(imgBuffer);
    }
    glGenTextures( 1, &texture_obj);
    glBindTexture( GL_TEXTURE_2D, texture_obj);

    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, texWidth,
                  texHeight, 0, GL_RGB,
                  GL_UNSIGNED_BYTE, texture );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glBindTexture(GL_TEXTURE_2D, 0);


    glBindVertexArray(vaos[OBJ]);

    glBindBuffer(GL_ARRAY_BUFFER, vbos[OBJ_UV_VBO]);
    glBufferData(GL_ARRAY_BUFFER, sts.size() * sizeof(VertexUV), sts.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindVertexArray(0);

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


    int f = 0;
    float d_time = 0;

    while (!glfwWindowShouldClose(window)) {
        // Set frame time
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        d_time += deltaTime;

        do_movement();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view;
        if (!thirdPerson) {
            view = camera.GetViewMatrix();
        } else {
            view = glm::lookAt(camera.Position - camera.Front - camera.Front + glm::vec3(0.0f, 3.0f, 0.0f), camera.Position + glm::vec3(0.0f, 1.7f, 0.0f), camera.Up);
        }
        glm::mat4 projection(glm::perspective(camera.Zoom, (float)WIDTH/HEIGHT, 0.2f, 100.0f));

        glUseProgram(mapProgram);
        glBindVertexArray(vaos[MAP]);

        {
            glm::mat4 view;
            view = glm::translate(view, glm::vec3(-0.7f, 0.7f, 0.0f));
            view = glm::scale(view, glm::vec3(0.5f, 0.5f, 0.5f));
            viewLoc = glGetUniformLocation(mapProgram, "view");
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
        }

        {
            GLint colorLoc = glGetUniformLocation(mapProgram, "inColor");
            glm::vec3 color(1.0f, 0.0f, 0.0f);
            glUniform3fv(colorLoc, 1, &color[0]);
            glm::mat4 model;
            model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
            model = glm::translate(model, glm::vec3(camera.Position.x, -camera.Position.z,0.0f));
            modelLoc = glGetUniformLocation(mapProgram, "model");
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        for (const auto& dot : dots) {
            GLint colorLoc = glGetUniformLocation(mapProgram, "inColor");
            glm::vec3 color(0.3f, 0.8f, 0.9f);
            glUniform3fv(colorLoc, 1, &color[0]);
            glm::mat4 model;
            model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
            model = glm::translate(model, glm::vec3(dot.x, -dot.z,0.0f));
            modelLoc = glGetUniformLocation(mapProgram, "model");
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        glBindVertexArray(0);

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
        modelLoc = glGetUniformLocation(program, "model");
        viewLoc = glGetUniformLocation(program, "view");
        projLoc = glGetUniformLocation(program, "projection");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);

        {
            glBindVertexArray(vaos[OBJ]);
            glm::mat4 model;
            model = glm::translate(model, glm::vec3(camera.Position.x, 0.0f, camera.Position.z));
            model = glm::translate(model, glm::vec3(0.0f, 1.3f, 0.0f));
            model = glm::scale(model, glm::vec3(0.03f, 0.03f, 0.03f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
            glBindTexture(GL_TEXTURE_2D, texture_obj);
            if (isStand) {
                if (f > 39) f = 0;
            } else {
                if (f > 45 || f < 40) f = 40;
            }
            glBindBuffer(GL_ARRAY_BUFFER, vbos[OBJ_VBO]);
            glBufferData(GL_ARRAY_BUFFER, frameVertices[f].size() * sizeof(Vertex), frameVertices[f].data(),
                         GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

            glDrawArrays(GL_TRIANGLES, 0, frameVertices[f].size());
            if (d_time > 0.1) {
                f++;
                d_time = 0;
            }
            glBindVertexArray(0);
        }

        glBindVertexArray(vaos[MAIN]);

        glBindTexture(GL_TEXTURE_2D, wallTexture);

        for (int j = 0; j < 4; j++) {
            for (const auto &dot : dots) {
                glm::mat4 model;
                model = glm::translate(model, glm::vec3(dot.x, 1.0f * j + 1, dot.z));
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }

        }
//        for (int j = 0; j < 2; j++) {
//            {
//                glm::mat4 model;
//                model = glm::translate(model, glm::vec3(camera.Position.x, 1.0f * j + 1, camera.Position.z));
//                model = glm::rotate(model, glm::radians(camera.Yaw), glm::vec3(0.0f, -1.0f, 0.0f));
//                model = glm::scale(model, glm::vec3(0.25f, 1.0f, 0.25f));
//                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
//                glDrawArrays(GL_TRIANGLES, 0, 36);
//            }
//        }

        glBindTexture(GL_TEXTURE_2D, floorTexture);

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

    if (action == GLFW_PRESS && key == GLFW_KEY_P) {
        thirdPerson = !thirdPerson;
    }
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

    // stop fly
    camera.ProcessMouseMovement(xoffset, 0);
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
    faces.push_back(resource(right.jpg));
    faces.push_back(resource(left.jpg));
    faces.push_back(resource(top.jpg));
    faces.push_back(resource(bottom.jpg));
    faces.push_back(resource(back.jpg));
    faces.push_back(resource(front.jpg));
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

    mapProgram = glCreateProgram();
    GLuint mapVShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint mapFShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(mapVShader, 1, &glsl::mapVShader, NULL);
    glShaderSource(mapFShader, 1, &glsl::mapFShader, NULL);
    glCompileShader(mapVShader);
    glCompileShader(mapFShader);
    glAttachShader(mapProgram, mapVShader);
    glAttachShader(mapProgram, mapFShader);
    glLinkProgram(mapProgram);
    glDeleteShader(mapVShader);
    glDeleteShader(mapFShader);

    GLfloat mapVertices[] = {
            -0.5f, 0.5f, 0.0f,
            0.5f, 0.5f, 0.0f,
            -0.5f, -0.5f, 0.0f,

            -0.5f, -0.5f, 0.0f,
            0.5f, 0.5f, 0.0f,
            0.5f, -0.5f, 0.0f
    };

    glBindVertexArray(vaos[MAP]);

    glBindBuffer(GL_ARRAY_BUFFER, vbos[DOT]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(mapVertices), mapVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, 0, 0);

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
