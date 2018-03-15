//
// Created by 何昊 on 2018/03/05.
//

#include "Ocean.h"

#include <random>
#include <iostream>
#include <vector>

static const float PI = 3.1415926f;

static void bitReverseCopy(const std::vector<std::complex<float>> &a,
                           std::vector<std::complex<float>> &A)
{
    auto n = (int)A.size();
    for (int i = 0; i < n; ++i) {
        // 0b001 -> 0b100
        int revi = i;
        int len = 1;
        for (int j = 0; j < 32; ++j) {
            if ((1 << j) == n) {
                len = j;
                break;
            }
        }
        for (int j = 0; j < len/2; ++j) {
            int tmp1 = (revi >> j) & 0x1;
            int tmp2 = (revi >> (len - 1 - j)) & 0x1;
            revi = revi & ~(1 << j);
            revi = revi | (tmp2 << j);
            revi = revi & ~(1 << (len - 1 - j));
            revi = revi | (tmp1 << (len - 1 - j));
        }
        //cout << i << " " << revi << endl;
        A[revi] = a[i];
    }
}

static std::vector<std::complex<float>> iterativeFFT(const std::vector<std::complex<float>> &a)
{
    using namespace std;
    auto n = a.size();
    vector<complex<float>> A(n);
    bitReverseCopy(a, A);
    for (int s = 1; (1 << s) <= n; ++s) {
        auto m = (1 << s);
        auto wm = complex<float>(cos(2*PI/m), sin(2*PI/m));
        for (int k = 0; k < n; k += m) {
            complex<float> w = 1.0;
            for (int j = 0; j < m/2; ++j) {
                auto t = w * A[k + j + m/2];
                auto u = A[k + j];
                A[k + j] = u + t;
                A[k + j + m/2] = u - t;
                w = w * wm;
            }
        }
    }
    return A;
}


Ocean::Ocean(glm::vec2 wind, int resolution, float amplitude)
        : w(wind), N(resolution), A(amplitude)
{
    useFFT = true;
    g = 9.8f;
    PI = 3.1415926f;
    L = N / 8;
    unitWidth = 5.0f;
    choppy = 0.0f;
    vertexCount = normalCount = 3 * N * N;
    indexCount  = 6 * N * N;
    vertices = new float[vertexCount];
    normals  = new float[normalCount];
    indices  = new unsigned int[indexCount];
    hBuffer            = new std::complex<float>[N * N];
    kBuffer            = new glm::vec2[N * N];
    epsilonBufferx     = new std::complex<float>[N * N];
    epsilonBuffery     = new std::complex<float>[N * N];
    displacementBuffer = new glm::vec2[N * N];
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
    delete[] hBuffer;
    delete[] kBuffer;
    delete[] epsilonBufferx;
    delete[] epsilonBuffery;
    delete[] displacementBuffer;
}

void Ocean::generateWave(float time)
{
    // Eliminate inital status when time accumulate from 0
    time += 10000;
    using namespace std;
    // Compute h buffer
    for (int n = -N / 2; n < N / 2; ++n) {
        for (int m = -N / 2; m < N / 2; ++m) {
            glm::vec2 k = glm::vec2(2.0f * PI * n / L, 2.0f * PI * m / L);
            int bufferIndex = (n + N/2) * N + m + N/2;
            kBuffer[bufferIndex] = k;
            hBuffer[bufferIndex] = h(k, time);

            epsilonBufferx[bufferIndex] = hBuffer[bufferIndex] * complex<float>(0.0f, kBuffer[bufferIndex].x);
            epsilonBuffery[bufferIndex] = hBuffer[bufferIndex] * complex<float>(0.0f, kBuffer[bufferIndex].y);
        }
    }

    // Set Wave vertices and normals seperately
    if (useFFT) {
        auto *HBuffer = new std::complex<float>[N * N];

        for (int i = 0; i < N; ++i) {
            vector<complex<float>> a(N);

            // Vertex 1D
            for (int j = 0; j < N; ++j)
                a[j] = hBuffer[i * N + j];
            auto buf = iterativeFFT(a);
            for (int j = 0; j < N; ++j)
                HBuffer[i * N + j] = buf[j];

            vector<complex<float>> b(N), c(N);
            for (int j = 0; j < N; ++j) {
                b[j] = epsilonBufferx[i * N + j];
                c[j] = epsilonBuffery[i * N + j];
            }
            auto buf2 = iterativeFFT(b), buf3 = iterativeFFT(c);
            for (int j = 0; j < N; ++j) {
                epsilonBufferx[i * N + j] = buf2[j];
                epsilonBuffery[i * N + j] = buf3[j];
            }
        }

        for (int i = 0; i < N; ++i) {
            vector<complex<float>> a(N);
            for (int j = 0; j < N; ++j)
                a[j] = HBuffer[j * N + i];
            auto buf = iterativeFFT(a);
            for (int j = 0; j < N; ++j) {
                if ((i + j) % 2 == 0)
                    HBuffer[j * N + i] = buf[j];
                else
                    HBuffer[j * N + i] = -buf[j];
            }

            vector<complex<float>> b(N), c(N);
            for (int j = 0; j < N; ++j) {
                b[j] = epsilonBufferx[j * N + i];
                c[j] = epsilonBuffery[j * N + i];
            }
            auto buf2 = iterativeFFT(b), buf3 = iterativeFFT(c);
            for (int j = 0; j < N; ++j) {
                if ((i + j) % 2 == 0) {
                    epsilonBufferx[j * N + i] = buf2[j];
                    epsilonBuffery[j * N + i] = buf3[j];
                } else {
                    epsilonBufferx[j * N + i] = -buf2[j];
                    epsilonBuffery[j * N + i] = -buf3[j];
                }
            }
        }

        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < N; ++j) {
                int pos = 3 * (i * N + j);
                float x = unitWidth * L * (i - N / 2.0f) / N,
                      z = unitWidth * L * (j - N / 2.0f) / N;
                vertices[pos + 0] = x;
                vertices[pos + 1] = HBuffer[i * N + j].real();
                vertices[pos + 2] = z;

                normals[pos + 0] = -epsilonBufferx[i * N + j].real();
                normals[pos + 1] = 1;
                normals[pos + 2] = -epsilonBuffery[i * N + j].real();
            }
        }
        delete[] HBuffer;
    } else {
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
}

float Ocean::H(float x, float z, float t)
{
    using std::complex;
    complex<float> result(0.0f, 0.0f);
    for (int n = -N / 2; n < N / 2; ++n) {
        for (int m = -N / 2; m < N / 2; ++m) {
            int bufferIndex = (n + N/2) * N + m + N/2;
            glm::vec2 k = kBuffer[bufferIndex];
            float k_dot_x = glm::dot(k, glm::vec2(x, z));

            result += hBuffer[bufferIndex] * complex<float>(cos(k_dot_x), sin(k_dot_x));
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
    return dist(generator);
}

float Ocean::Ph(glm::vec2 k)
{
    if (glm::length(k) < 0.001f) return 0.0f;
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
        for (int m = -N / 2; m < N / 2; ++m) {
            int bufferIndex = (n + N/2) * N + m + N/2;
            glm::vec2 k = kBuffer[bufferIndex];
            float k_dot_x = glm::dot(glm::vec2(x, z), k);

            complex<float> tmp = hBuffer[bufferIndex]
                                 * complex<float>(cos(k_dot_x), sin(k_dot_x));

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
        for (int m = -N / 2; m < N / 2; ++m) {
            int bufferIndex = (n + N/2) * N + m + N/2;
            glm::vec2 k = kBuffer[bufferIndex];
            float k_dot_x = glm::dot(glm::vec2(x, z), k);

            complex<float> tmp = hBuffer[bufferIndex]
                                 * complex<float>(sin(k_dot_x), -cos(k_dot_x));

            glm::vec2 v = (tmp.real()/glm::length(k)) * k;
            result.x += v.x;
            result.z += v.y;
        }
    }
    return result;
}








