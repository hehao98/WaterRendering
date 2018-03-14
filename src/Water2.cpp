//
// Created by 何昊 on 2018/03/05.
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
#include "TextRenderer.h"

// Water related header file
#include "Ocean.h"

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

    // Load shaders
    Shader shader("shaders/SingleColor.vert", "shaders/WaterWireframe.frag");
    Shader textShader("shaders/TextShader.vert", "shaders/TextShader.frag");

    // Initialize fonts
    TextRenderer textRenderer;

    // Initialize skybox
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

    // Necessary OpenGL Parameters
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    gCamera.Position = glm::vec3(0.0f, 10.0f, 20.0f);

    Ocean ocean(glm::vec2(2.0f, 2.0f), 16, 0.2f);
    ocean.generateWave((float)glfwGetTime());
    // Pass the vertex data to GPU
    unsigned int VBO, VBO2, EBO, VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * ocean.vertexCount, ocean.vertices, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glGenBuffers(1, &VBO2);
    glBindBuffer(GL_ARRAY_BUFFER, VBO2);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * ocean.normalCount, ocean.normals, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * ocean.indexCount, ocean.indices, GL_STATIC_DRAW);

    std::cout << "Total vertices: " << ocean.vertexCount << std::endl;
    for (int i = 0; i < 50; ++i) {
        using namespace std;
        cout << "(" << ocean.vertices[3 * i] << ","
             << ocean.vertices[3 * i + 1] << ","
             <<  ocean.vertices[3 * i + 2] << ")";
        cout << "(" << ocean.indices[3 * i] << ","
             << ocean.indices[3 * i + 1] << ","
             <<  ocean.indices[3 * i + 2] << ")";
        cout << "(" << ocean.normals[3 * i] << ","
             << ocean.normals[3 * i + 1] << ","
             << ocean.normals[3 * i + 2] << ")" << endl;
    }

    // Game loop
    while (!glfwWindowShouldClose(window)) {
        // Calculate how much time since last frame
        auto currentFrame = (float)glfwGetTime();
        gDeltaTime = currentFrame - gLastFrame;
        gLastFrame = currentFrame;
        auto currentFPS = (int)(1.0f / gDeltaTime);

        // Handle user input
        processInput(window);

        // Update wave data
        ocean.generateWave((float)glfwGetTime());
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * ocean.vertexCount, ocean.vertices, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, VBO2);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * ocean.normalCount, ocean.normals, GL_DYNAMIC_DRAW);

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
        shader.setVec4("color", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
        glBindVertexArray(VAO);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawElements(GL_TRIANGLES, ocean.indexCount, GL_UNSIGNED_INT, nullptr);

        // Start to render texts
        //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        textRenderer.projection = glm::ortho(0.0f, (float)gScreenWidth,
                                             0.0f, (float)gScreenHeight);
        textRenderer.renderText(textShader, "FPS: " + std::to_string(currentFPS),
                                0.0f, gScreenHeight - 48.0f*0.3f, 0.3f,
                                glm::vec3(0.0, 1.0f, 1.0f));
        textRenderer.renderText(textShader, "Use WSAD to move, mouse to look around",
                                0.0f, 3.0f, 0.3f,
                                glm::vec3(0.0, 1.0f, 1.0f));
        // Rendering Ends here

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
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