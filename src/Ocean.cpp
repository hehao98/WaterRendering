//
// Created by 何昊 on 2018/03/05.
//

#include "Ocean.h"

#include <random>
#include <iostream>

Ocean::Ocean(glm::vec2 wind, int resolution, float amplitude)
        : w(wind), N(resolution), A(amplitude)
{
    g = 9.8f;
    PI = 3.1415926f;
    L = 10;
    unitWidth = 1.0f;
    choppy = 0.0f;
    vertexCount = normalCount = 3 * N * N;
    indexCount  = 6 * N * N;
    vertices = new float[vertexCount];
    normals  = new float[normalCount];
    indices  = new unsigned int[indexCount];
    // Precompute indices
    for (unsigned int i = 0; i < N - 1; ++i) {
        for (unsigned int j = 0; j < N - 1; ++j) {
            indices[6 * (i * N + j)    ] = (i * N + j);
            indices[6 * (i * N + j) + 1] = (i * N + j + 1);
            indices[6 * (i * N + j) + 2] = ((i + 1) * N + j);
            indices[6 * (i * N + j) + 3] = (i * N + j + 1);
            indices[6 * (i * N + j) + 4] = ((i + 1) * N + j);
            indices[6 * (i * N + j) + 5] = ((i + 1) * N + j + 1);
        }
    }
}

Ocean::~Ocean()
{
    delete[] vertices;
    delete[] normals;
    delete[] indices;
}

void Ocean::generateWave(float time)
{
    // Set Wave vertices and normals seperately
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            int pos = 3 * (i * N + j);

            float x = unitWidth * L * (i - N / 2.0f) / N,
                    z = unitWidth * L * (j - N / 2.0f) / N;

            // Displacement vector
            //glm::vec3 d = glm::vec3(0.0f,0.0f,0.0f);
            glm::vec3 d = choppy * D(x, z, time);

            vertices[pos + 0] = x + d.x;
            vertices[pos + 1] = H(x, z, time) + d.y;
            vertices[pos + 2] = z + d.z;

            // Epsilon vector for calculating normals
            glm::vec3 e = epsilon(x, z, time);

            normals[pos + 0] = -e.x;
            normals[pos + 1] = 1;
            normals[pos + 2] = -e.z;
        }
    }
}

float Ocean::H(float x, float z, float t)
{
    using std::complex;
    complex<float> result(0.0f, 0.0f);
    for (int n = -N / 2; n < N / 2; ++n) {
        for (int m = -N < 2; m < N / 2; ++m) {
            glm::vec2 k = glm::vec2(2.0f * PI * n / L, 2.0f * PI * m / L);
            float k_dot_x = glm::dot(k, glm::vec2(x, z));

            result += h(k, t) * complex<float>(cos(k_dot_x), sin(k_dot_x));
        }
    }
    return result.real();
}

std::complex<float> Ocean::h(glm::vec2 k, float t)
{
    using std::complex;
    complex<float> result(0.0f, 0.0f);
    float omega_k = omega(k);
    float coswt = cos(omega_k * t);
    float sinwt = sin(omega_k * t);
    result += h0(k) * complex<float>(coswt, sinwt);
    result += std::conj(h0(-k)) * complex<float>(coswt, -sinwt);
    return result;
}

std::complex<float> Ocean::h0(glm::vec2 k)
{
    using std::complex;
    float xi1 = normalRandom(), xi2 = normalRandom();
    return (1.0f/sqrt(2.0f)) * complex<float>(xi1, xi2) * sqrt(Ph(k));
}

float Ocean::normalRandom()
{
    static auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    static std::default_random_engine generator((unsigned)seed);
    static std::normal_distribution<float> dist(0.5, 0.1);
    return glm::clamp(dist(generator), 0.0f, 1.0f);
}

float Ocean::Ph(glm::vec2 k)
{
    if (glm::length(k) < 0.01f) return 0.0f;
    float absk = glm::length(k);
    float L = glm::length(w)*glm::length(w) / g;
    float result = A;
    result *= exp(-1.0f/((absk*L)*(absk*L))) / pow(absk, 4);
    result *= pow(glm::dot(glm::normalize(k), glm::normalize(w)), 2);
    return result;
}

float Ocean::omega(glm::vec2 k)
{
    float klen = glm::length(k);
    return sqrt(g * klen);
}

glm::vec3 Ocean::epsilon(float x, float z, float t)
{
    using std::complex;
    glm::vec3 result(0.0f, 0.0f, 0.0f);
    for (int n = -N / 2; n < N / 2; ++n) {
        for (int m = -N < 2; m < N / 2; ++m) {
            glm::vec2 k = glm::vec2(2.0f * PI * n / L, 2.0f * PI * m / L);
            float k_dot_x = glm::dot(glm::vec2(x, z), k);

            complex<float> tmp = h(k, t) * complex<float>(cos(k_dot_x), sin(k_dot_x));

            glm::vec2 v = -tmp.imag() * k;
            result.x += v.x;
            result.z += v.y;
        }
    }
    return result;
}

glm::vec3 Ocean::D(float x, float z, float t)
{
    using std::complex;
    glm::vec3 result(0.0f, 0.0f, 0.0f);
    for (int n = -N / 2; n < N / 2; ++n) {
        for (int m = -N < 2; m < N / 2; ++m) {
            glm::vec2 k = glm::vec2(2.0f * PI * n / L, 2.0f * PI * m / L);
            float k_dot_x = glm::dot(glm::vec2(x, z), k);

            complex<float> tmp = h(k, t) * complex<float>(sinf(k_dot_x), -cosf(k_dot_x));

            glm::vec2 v = (tmp.real()/glm::length(k)) * k;
            result.x += v.x;
            result.z += v.y;
        }
    }
    return result;
}








