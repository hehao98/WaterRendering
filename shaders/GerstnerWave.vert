#version 330 core

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
struct Wave {
    float Q;
    float A;
    vec2 D;
    float l;
    float s;
};

layout (location = 0) in vec3 aPos;

out VS_OUT {
    vec4 fragPos;
    vec3 normal;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;
uniform int waveCount;
uniform Wave waves[10];

void main()
{
    vec3 pos = aPos;
    // Save the original vertex position as parameters of the equation
    float u = pos.x, v = pos.z;
    // Calculate vertex position
    for (int i = 0; i < waveCount; ++i) {
        //float w = sqrt(2 * 3.14 * 9.8 / waves[i].l);
        float w = 2 * 3.14 / waves[i].l;
        float fi = 2 * 3.14 * waves[i].s / waves[i].l;
        pos.x += waves[i].Q * waves[i].A * waves[i].D.x
                 * cos(dot(waves[i].D, vec2(u, v)) * w + fi * time);
        pos.y += waves[i].A * sin(dot(waves[i].D, vec2(u, v)) * w + fi * time);
        pos.z += waves[i].Q * waves[i].A * waves[i].D.y
                 * cos(dot(waves[i].D, vec2(u, v)) * w + fi * time);
    }
    vs_out.fragPos = model * vec4(pos, 1.0);

    // Calculate vertex normal
    // Since the derivative of the sum is the sum of derivatives
    // We can directly add the normal components together to get the real normal
    vs_out.normal = vec3(0.0, 1.0, 0.0);
    for (int i = 0; i < waveCount; ++i) {
        //float w = sqrt(2 * 3.14 * 9.8 / waves[i].l);
        float w = 2 * 3.14 / waves[i].l;
        float fi = 2 * 3.14 * waves[i].s / waves[i].l;
        vs_out.normal.x += (-waves[i].D.x) * w * waves[i].A
                           * cos(dot(waves[i].D, vec2(u, v)) * w + fi * time);
        vs_out.normal.y -= waves[i].Q * w * waves[i].A * sin(dot(waves[i].D, vec2(u, v)) * w + fi * time);
        vs_out.normal.z += (-waves[i].D.y) * w * waves[i].A
                           * cos(dot(waves[i].D, vec2(u, v)) * w + fi * time);
    }
    // Transform the normal into world coordinate
    vs_out.normal = mat3(transpose(inverse(model))) * vs_out.normal;

    gl_Position = projection * view * model * vec4(pos, 1.0);
}
