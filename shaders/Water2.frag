#version 330 core

in VS_OUT {
    vec4 fragPos;
    vec3 normal;
    vec2 texCoord;
} fs_in;

out vec4 fragColor;

uniform vec3 viewPos;
uniform vec3 deepWaterColor;
uniform vec3 shallowWaterColor;
uniform sampler2D normalMap;
uniform sampler2D heightMap;

void main()
{
    //vec3 n = normalize(fs_in.normal);
    vec3 n = normalize(2.0f * vec3(texture(normalMap, fs_in.texCoord)) - 1.0f);
    vec3 eyeVec = normalize(viewPos - vec3(fs_in.fragPos));
    float facing = clamp(dot(normalize(vec3(1.0, 1.0, 1.0)), n), 0.0, 1.0);
    fragColor = vec4(mix(shallowWaterColor, deepWaterColor, facing), 1.0);
}