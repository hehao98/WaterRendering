//
// Created by 何昊 on 2018/03/02.
//

#include "Waves.h"

#include <string>
#include <iostream>
#include <algorithm>

// GLAD: A library that wraps OpenGL functions to make things easier
//       Note that GLAD MUST be included before GLFW
#include "glad/glad.h"
// GLFW: A library that helps us manage windows
#include <GLFW/glfw3.h>

static inline float randf(float min, float max, int precision = 1000)
{
    if (min > max) std::swap(min, max);
    float delta = max - min;
    auto i = int(delta * precision);
    return ((float)(rand() % i) / (float)precision) + min;
}

static inline int randi(int min, int max)
{
    if (min > max) std::swap(min, max);
    return (rand() % (max - min)) + min;
}

void setGersterWaveData(Shader &shader, glm::vec2 windDir, int waveCount, GerstnerWave *waves)
{
    srand((unsigned int)time(nullptr));
    int r = rand() % 100;
    windDir = glm::vec2(cosf(r), sinf(r));
    shader.use();
    shader.setInt("waveCount", waveCount);
    for (int i = 0; i < waveCount; ++i) {
        waves[i].A = randf(0.01f, 0.05f);
        shader.setFloat("waves[" + std::to_string(i) + "].A", waves[i].A);

        waves[i].Q = randf(0.3f, 0.4f);
        shader.setFloat("waves[" + std::to_string(i) + "].Q", waves[i].Q);

        // The wave direction is determined by wind direction
        // but have a random angle to the wind direction
        float windAngle = acosf((windDir.x/sqrtf(windDir.x*windDir.x + windDir.y*windDir.y)));
        if (windDir.y < 0) windAngle = -windAngle;
        float waveAngle = randf(windAngle - glm::radians(60.0f),
                                windAngle + glm::radians(60.0f));
        waves[i].D.x = cos(waveAngle);
        waves[i].D.y = sin(waveAngle);
        shader.setVec2("waves[" + std::to_string(i) + "].D", waves[i].D);

        waves[i].s = randf(0.5f, 1.0f);
        shader.setFloat("waves[" + std::to_string(i) + "].s", waves[i].s);

        waves[i].l = waves[i].A * randf(30.0f, 60.0f);
        shader.setFloat("waves[" + std::to_string(i) + "].l", waves[i].l);
    }
}

unsigned int genGersterWaveTexture(glm::vec2 windDir, float size, int n)
{
    // Use the Texture on a 10m basis
    GerstnerWave wave;
    wave.A = randf(0.01f, 0.02f);
    wave.Q = randf(0.3f, 0.4f);
    // The wave direction is determined by wind direction
    // but have a random angle to the wind direction
    float windAngle = acosf((windDir.x/sqrtf(windDir.x*windDir.x + windDir.y*windDir.y)));
    if (windDir.y < 0) windAngle = -windAngle;
    float waveAngle = randf(windAngle - glm::radians(45.0f),
                            windAngle + glm::radians(45.0f));
    wave.D.x = cos(waveAngle);
    wave.D.y = sin(waveAngle);
    wave.s = randf(0.5f, 1.0f);
    wave.l = ((float)randi(4, 32) / n);


    auto *data = new float[3 * n * n];
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            glm::vec3 normal = glm::vec3(0.0f, 1.0f, 0.0f);
            float time = (float)glfwGetTime();
            float u = (float) j / n, v = (float) i / n;
            //float w = sqrt(2 * 3.14 * 9.8 / waves[i].l);
            float w = 2 * 3.14f / wave.l;
            float fi = 2 * 3.14f * wave.s / wave.l;

            normal.x += (-wave.D.x) * w * wave.A
                        * cos(glm::dot(wave.D, glm::vec2(u, v)) * w + fi * time);
            normal.y -= wave.Q * w * wave.A * sin(dot(wave.D, glm::vec2(u, v)) * w + fi * time);
            normal.z += (-wave.D.y) * w * wave.A
                        * cos(glm::dot(wave.D, glm::vec2(u, v)) * w + fi * time);

            data[3 * (i * n + j) + 0] = normal.x;
            data[3 * (i * n + j) + 1] = normal.y;
            data[3 * (i * n + j) + 2] = normal.z;
        }
    }

    // Pass the data to OpenGL
    unsigned int tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    // Set default texture wrapping/filtering options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, n, n,
                 0, GL_RGB, GL_FLOAT, data);
    delete[] data;
    return tex;
}

void setSineWaveData(Shader &shader, int waveCount, SineWave *waves)
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

void updateSineWaveData(Shader &shader, int waveCount, SineWave *waves, float deltaTime)
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
                waves[i].amp -= waves[i].changeRate * deltaTime;
                shader.setFloat("waves[" + std::to_string(i) + "].amp", waves[i].amp);
            } else {
                waves[i].amp += waves[i].changeRate * deltaTime;
                shader.setFloat("waves[" + std::to_string(i) + "].amp", waves[i].amp);
                if (waves[i].amp > waves[i].maxAmp) {
                    waves[i].rising = false;
                }
            }
        }
    }
}