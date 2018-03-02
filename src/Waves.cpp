//
// Created by 何昊 on 2018/03/02.
//

#include "Waves.h"

#include <string>
#include <iostream>
#include <algorithm>

inline float randf(float min, float max, int precision = 1000)
{
    if (min > max) std::swap(min, max);
    float delta = max - min;
    auto i = int(delta * precision);
    return ((float)(rand() % i) / (float)precision) + min;
}

void setGersterWaveData(Shader &shader, glm::vec2 windDir, int waveCount, GerstnerWave *waves)
{
    srand((unsigned int)time(nullptr));
    windDir = glm::normalize(windDir);
    shader.use();
    shader.setInt("waveCount", waveCount);
    for (int i = 0; i < waveCount; ++i) {
        waves[i].A = randf(0.05f, 0.1f);
        shader.setFloat("waves[" + std::to_string(i) + "].A", waves[i].A);

        waves[i].Q = randf(0.3f, 0.4f);
        shader.setFloat("waves[" + std::to_string(i) + "].Q", waves[i].Q);

        // The wave direction is determined by wind direction
        // but have a random angle to the wind direction
        float windAngle = acosf((windDir.x/sqrtf(windDir.x*windDir.x + windDir.y*windDir.y)));
        if (windDir.y < 0) windAngle = -windAngle;
        float waveAngle = randf(windAngle - glm::radians(45.0f),
                                windAngle + glm::radians(45.0f));
        waves[i].D.x = cos(waveAngle);
        waves[i].D.y = sin(waveAngle);
        shader.setVec2("waves[" + std::to_string(i) + "].D", waves[i].D);

        waves[i].s = randf(2.0f, 4.0f);
        shader.setFloat("waves[" + std::to_string(i) + "].s", waves[i].s);

        // Wave length range: [4.0f, 16.0f] (Metric: meter)
        waves[i].l = waves[i].A * randf(80.0f, 160.0f);
        shader.setFloat("waves[" + std::to_string(i) + "].l", waves[i].l);
    }
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