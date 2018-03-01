#version 330 core

// W(x, z, t) = A * sin(dot(D, xz) * omega + t * fi)
// where omega = 2 / wavelen, fi = 2 * speed / wavelen
struct SineWave {
    float amp;
    vec2 dir;
    float wavelen;
    float speed;
};

layout (location = 0) in vec3 aPos;

out VS_OUT {
    vec3 fragPos;
    vec3 normal;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;
uniform int waveCount;
uniform SineWave waves[10];

void main()
{
    // Calculate vertex position
    vec3 pos = aPos;
    for (int i = 0; i < waveCount; ++i) {
        float omega = 2 / waves[i].wavelen;
        float fi = 2 * waves[i].speed / waves[i].wavelen;
        pos.y += waves[i].amp * sin(dot(waves[i].dir, pos.xz) * omega + time * fi);
    }
    vs_out.fragPos = pos;

    // Calculate vertex normal
    // Given surface W(x, z, t) - y = 0
    // its normal is ( -A * dir.x * omega * cos(dot(D, xz) * omega + t * fi),
    //                 1,
    //                 -A * dir.y * omega * cos(dot(D, xz) * omega + t * fi) )
    // Since the derivative of the sum is the sum of derivatives
    // We can directly add the normal components together to get the real normal
    for (int i = 0; i < waveCount; ++i) {
        float omega = 2 / waves[i].wavelen;
        float fi = 2 * waves[i].speed / waves[i].wavelen;
        vec3 n;
        n.x = -waves[i].amp * waves[i].dir.x * omega
              * cos(dot(waves[i].dir, pos.xz) * omega + time * fi);
        n.y = 1;
        n.z = -waves[i].amp * waves[i].dir.y * omega
              * cos(dot(waves[i].dir, pos.xz) * omega + time * fi);
        vs_out.normal += n;
    }
    vs_out.normal = normalize(vs_out.normal);

    gl_Position = projection * view * model * vec4(pos, 1.0);
}
