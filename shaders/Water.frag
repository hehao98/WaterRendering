#version 330 core

in VS_OUT {
    vec4 fragPos;
    vec3 normal;
} fs_in;

out vec4 fragColor;

uniform vec3 viewPos;
uniform vec3 deepWaterColor;
uniform vec3 shallowWaterColor;

void main()
{
    vec3 eyeVec = normalize(viewPos - vec3(fs_in.fragPos));
    float facing = clamp(dot(eyeVec, normalize(fs_in.normal)), 0.0, 1.0);
    fragColor = vec4(mix(shallowWaterColor, deepWaterColor, facing), 1.0);
}