#version 330 core

in VS_OUT {
    vec4 fragPos;
    vec3 normal;
} fs_in;

out vec4 fragColor;

uniform vec3 viewPos;
uniform vec3 deepWaterColor;
uniform vec3 shallowWaterColor;
// Wave normals described in texture
uniform int waveMapCount;
uniform sampler2D waveMaps[5];

void main()
{
    vec3 n = fs_in.normal;
    for (int i = 0; i < waveMapCount; ++i)
        n += vec3(texture(waveMaps[i], fs_in.fragPos.xz / 10.0));
    vec3 eyeVec = normalize(viewPos - vec3(fs_in.fragPos));
    float facing = clamp(dot(normalize(vec3(1.0, 1.0, 1.0)), normalize(n)), 0.0, 1.0);
    fragColor = vec4(mix(shallowWaterColor, deepWaterColor, facing), 1.0);
    //fragColor = vec4(n, 1.0);
}