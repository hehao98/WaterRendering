//
// Provide simple interface to generate waves and set wave data
//
// Created by 何昊 on 2018/03/02.
//

#ifndef PROJECT_WAVES_H
#define PROJECT_WAVES_H

// GLM Math Library
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"

// A Gerstner Wave is defined as: (Parameter is u and v)
// x = u + Q * A * D.x * cos(w * D.x * u + w * D.y * v + fi * t)
// y = A * sin(w * D.x * u + w * D.y * v + fi * t);
// z = v + Q * A * D.y * cos(w * D.x * u + w * D.y * v + fi * t)
// Where:
// u and v are original (x, y) position of a vertex,
// Q controls the sharpness of wave top, usually between 0 and 1,
// A is the maximum height of wave,
// D is the direction of the wave,
// w = sqrt(2 * pi * g / l), l is wave length, w is frequency, g = 9.8m/s^2,
// fi = 2 * pi * s / l, where s is speed
struct GerstnerWave
{
    float Q;
    float A;
    glm::vec2 D;
    float l;
    float s;
};

void setGersterWaveData(Shader &shader, int waveCount, GerstnerWave *waves);


// A Sine Wave is defined as:
// W(x, z, t) = A * sin(dot(D, xz) * omega + t * fi)
// where omega = 2 / wavelen, fi = 2 * speed / wavelen
struct SineWave
{
    float maxAmp;
    float amp;
    glm::vec2 dir;
    float wavelen;
    float speed;
    bool rising;
    float changeRate;
};

// Randomly set the SineWave data parameters
// And synchronize with the SineWave data in the shader
void setSineWaveData(Shader &shader, int waveCount, SineWave *waves);

// Update the SineWave data parameters per frame
// And synchronize with the SineWave data in the shader
void updateSineWaveData(Shader &shader, int waveCount, SineWave *waves, float deltaTime);


#endif //PROJECT_WAVES_H
