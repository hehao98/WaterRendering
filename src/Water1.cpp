//
// Created by Hao He on 2018-3-1.
//

#include <iostream>
#include <algorithm>

// GLM Math Library
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// GLAD: A library that wraps OpenGL functions to make things easier
//       Note that GLAD MUST be included before GLFW
#include "glad/glad.h"
// GLFW: A library that helps us manage windows
#include <GLFW/glfw3.h>

// Wrapper classes to make things a little easier
#include "Shader.h"
#include "Camera.h"
#include "Skybox.h"

// A Sine Wave is defined as
// W(x, y, t) = A * sin(dot(D, xz) * omega + t * fi)
// where omega = 2 / wavelen, fi = 2 * speed / wavelen
struct SineWave {
    float maxAmp;
    float amp;
    glm::vec2 dir;
    float wavelen;
    float speed;
    bool rising;
    float changeRate;
};


// **********GLFW window related functions**********
// Returns pointer to a initialized window with OpenGL context set up
GLFWwindow *init();
// Sometimes user might resize the window. so the OpenGL viewport should be adjusted as well.
void frameBufferSizeCallback(GLFWwindow *window, int width, int height);
// User input is handled in this function
void processInput(GLFWwindow *window);
// Mouse input is handled in this function
void mouseCallback(GLFWwindow *window, double xpos, double ypos);
void scrollCallback(GLFWwindow *window, double offsetX, double offsetY);

// Generate vertex data for a water surface
void genWaterVertexBuffer(int width, float *vertices, unsigned int *indices);
// Pass Wave Configuration to shader
void setWaveData(Shader shader, int waveCount, SineWave *waves);
void updateWaveData(Shader shader, int waveCount, SineWave *waves);
// Generate a cube map using the 6 file paths in the vector,
// Sequence: Right, left, top, bottom, back, front
unsigned int generateCubeMap(std::vector<std::string> facePaths);

// **********GLFW window related attributes**********
int gScreenWidth = 800;
int gScreenHeight = 600;
float gDeltaTime = 0.0f;
float gLastFrame = 0.0f;

Camera gCamera;

int main()
{
    GLFWwindow *window = init();
    if (window == nullptr) {
        std::cout << "Failed to initialize GLFW and OpenGL!" << std::endl;
        return -1;
    }

    Shader shader("shaders/SineWave.vert", "shaders/Water.frag");

    std::vector<std::string> skyboxPaths = {
            "textures/TropicalSunnyDay/TropicalSunnyDayLeft2048.png",
            "textures/TropicalSunnyDay/TropicalSunnyDayRight2048.png",
            "textures/TropicalSunnyDay/TropicalSunnyDayUp2048.png",
            "textures/TropicalSunnyDay/TropicalSunnyDayDown2048.png",
            "textures/TropicalSunnyDay/TropicalSunnyDayFront2048.png",
            "textures/TropicalSunnyDay/TropicalSunnyDayBack2048.png",
    };
    Shader skyboxShader("shaders/SkyboxShader.vert", "shaders/SkyboxShader.frag");
    Skybox skybox(skyboxPaths);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    gCamera.Position = glm::vec3(0.0f, 10.0f, 20.0f);

    // Initialize water surface vertex data
    int width = 100;
    int vertexCount = 3 * width * width;
    auto *vertices = new float[vertexCount];
    int indexCount = 2 * 3 * width * width;
    auto *indices = new unsigned int[indexCount];
    genWaterVertexBuffer(width, vertices, indices);
    // Pass the vertex data to GPU
    unsigned int VBO, EBO, VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertexCount, vertices, GL_STATIC_DRAW);
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indexCount, indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    int waveCount = 4;
    SineWave waves[10];
    setWaveData(shader, waveCount, waves);

    // Game loop
    while (!glfwWindowShouldClose(window)) {
        // Calculate how much time since last frame
        auto currentFrame = (float)glfwGetTime();
        gDeltaTime = currentFrame - gLastFrame;
        gLastFrame = currentFrame;

        // Handle user input
        processInput(window);

        // All the rendering starts from here
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Set up view and projection matrix
        glm::mat4 view = gCamera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(gCamera.Zoom),
                                                (float)gScreenWidth / gScreenHeight, 0.1f, 100.0f);


        //skybox.Draw(skyboxShader, view, projection);

        shader.use();
        // Set vertex shader data
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);
        shader.setMat4("model", glm::mat4(1.0f));
        shader.setFloat("time", (float)glfwGetTime());
        // Set fragment shader data
        shader.setVec3("viewPos", gCamera.Position);
        shader.setVec3("deepWaterColor", glm::vec3(0.1137f, 0.2745f, 0.4392f));
        shader.setVec3("shallowWaterColor", glm::vec3(0.45f, 0.55f, 0.7f));

        // updateWaveData(shader, waveCount, waves);
        glBindVertexArray(VAO);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        // Rendering Ends here

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    delete[] vertices;
    delete[] indices;
    glfwTerminate();
    return 0;
}

void genWaterVertexBuffer(int width, float *vertices, unsigned int *indices)
{
    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < width; ++j) {
            vertices[3 * (i * width + j)    ] = (i - width / 2.0f) / 4.0f;
            vertices[3 * (i * width + j) + 1] = 0;
            vertices[3 * (i * width + j) + 2] = (j - width / 2.0f) / 4.0f;
        }
    }
    for (unsigned int i = 0; i < width - 1; ++i) {
        for (unsigned int j = 0; j < width - 1; ++j) {
            indices[6 * (i * width + j)    ] = (i * width + j);
            indices[6 * (i * width + j) + 1] = (i * width + j + 1);
            indices[6 * (i * width + j) + 2] = ((i + 1) * width + j);
            indices[6 * (i * width + j) + 3] = (i * width + j + 1);
            indices[6 * (i * width + j) + 4] = ((i + 1) * width + j);
            indices[6 * (i * width + j) + 5] = ((i + 1) * width + j + 1);
        }
    }
}

void setWaveData(Shader shader, int waveCount, SineWave *waves)
{
    srand((unsigned int)time(nullptr));
    shader.use();
    shader.setInt("waveCount", waveCount);
    for (int i = 0; i < waveCount; ++i) {
        waves[i].maxAmp = ((rand() % 100) + 100.0f) / 4000.0f;
        waves[i].amp = waves[i].maxAmp;
        shader.setFloat("waves[" + std::to_string(i) + "].amp", waves[i].amp);
        waves[i].dir.x = (rand() % 1000) / 500.0f - 1.0f;
        waves[i].dir.y = powf(1 - waves[i].dir.x*waves[i].dir.x, 0.5);
        shader.setVec2("waves[" + std::to_string(i) + "].dir", waves[i].dir);
        waves[i].wavelen = (rand() % 1000 + 500) / 2000.0f;
        shader.setFloat("waves[" + std::to_string(i) + "].wavelen", waves[i].wavelen);
        waves[i].speed = (rand() % 3000 + 1000) / 10000.0f;
        shader.setFloat("waves[" + std::to_string(i) + "].speed", waves[i].speed);
        waves[i].changeRate = (rand() % 300) / 10000.0f + 0.005f;
        waves[i].rising = true;
    }
}



void updateWaveData(Shader shader, int waveCount, SineWave *waves)
{
    static const float EPSILON = 0.0001f;
    srand((unsigned int)time(nullptr));
    shader.use();
    shader.setInt("waveCount", waveCount);
    for (int i = 0; i < waveCount; ++i) {
        if (waves[i].amp < EPSILON && !waves[i].rising) {
            // Reset wave
            waves[i].maxAmp = ((rand() % 100) + 100.0f) / 4000.0f;
            waves[i].amp = 0.0f;
            shader.setFloat("waves[" + std::to_string(i) + "].amp", waves[i].amp);
            waves[i].dir.x = (rand() % 1000) / 500.0f - 1.0f;
            waves[i].dir.y = powf(1 - waves[i].dir.x * waves[i].dir.x, 0.5);
            shader.setVec2("waves[" + std::to_string(i) + "].dir", waves[i].dir);
            waves[i].wavelen = (rand() % 1000 + 500) / 2000.0f;
            shader.setFloat("waves[" + std::to_string(i) + "].wavelen", waves[i].wavelen);
            waves[i].speed = (rand() % 3000 + 1000) / 10000.0f;
            shader.setFloat("waves[" + std::to_string(i) + "].speed", waves[i].speed);
            waves[i].changeRate = (rand() % 300) / 10000.0f + 0.005f;
            waves[i].rising = true;
        }
        else {
            if (!waves[i].rising) {
                waves[i].amp -= waves[i].changeRate * gDeltaTime;
                shader.setFloat("waves[" + std::to_string(i) + "].amp", waves[i].amp);
            } else {
                waves[i].amp += waves[i].changeRate * gDeltaTime;
                shader.setFloat("waves[" + std::to_string(i) + "].amp", waves[i].amp);
                if (waves[i].amp > waves[i].maxAmp) {
                    waves[i].rising = false;
                }
            }
        }
    }
}

GLFWwindow *init()
{
    // Initialization of GLFW context
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // Something needed for Mac OS X
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Create a window object
    GLFWwindow *window = glfwCreateWindow(gScreenWidth, gScreenHeight, "Window Title", nullptr, nullptr);
    if (window == nullptr) {
        std::cout << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent(window);

    // Initialize GLAD before calling OpenGL functions
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return nullptr;
    }

    // Tell OpenGL the size of rendering window
    glViewport(0, 0, gScreenWidth * 2, gScreenHeight * 2);

    // Set the windows resize callback function
    glfwSetFramebufferSizeCallback(window, frameBufferSizeCallback);

    // Set up mouse input
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);

    return window;
}

void frameBufferSizeCallback(GLFWwindow *window, int width, int height)
{
    gScreenWidth = width;
    gScreenHeight = height;
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
    // Exit
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Handle camera movement
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        gCamera.ProcessKeyboard(FORWARD, gDeltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        gCamera.ProcessKeyboard(BACKWARD, gDeltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        gCamera.ProcessKeyboard(LEFT, gDeltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        gCamera.ProcessKeyboard(RIGHT, gDeltaTime);
    }
}

void mouseCallback(GLFWwindow *window, double xpos, double ypos)
{
    // Variables needed to handle mouse input
    static float lastMouseX = 400.0f;
    static float lastMouseY = 300.0f;
    static bool firstMouse = true;

    if (firstMouse) {
        lastMouseX = (float)xpos;
        lastMouseY = (float)ypos;
        firstMouse = false;
    }

    // Calculate mouse movement since last frame
    float offsetX = (float)xpos - lastMouseX;
    float offsetY = (float)ypos - lastMouseY;
    lastMouseX = (float)xpos;
    lastMouseY = (float)ypos;

    gCamera.ProcessMouseMovement(offsetX, offsetY);
}

void scrollCallback(GLFWwindow *window, double offsetX, double offsetY)
{
    gCamera.ProcessMouseScroll((float)offsetY);
}