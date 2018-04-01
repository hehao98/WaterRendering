#version 330 core

layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform sampler2D heightMap;
uniform sampler2D normalMap;

out VS_OUT {
    vec4 fragPos;
    vec3 normal;
    vec2 texCoord;
} vs_out;

void main()
{
    vec3 height = (vec3(texture(heightMap, aPos.xz / 50.0f)) - vec3(0.5f)) * 2.0f;
    vec3 pos = aPos + height;
    gl_Position = projection * view * model * vec4(pos, 1.0);
    vs_out.fragPos = model * vec4(pos, 1.0);
    vs_out.texCoord = aPos.xz / 50.0f;

    vec3 n = (vec3(texture(normalMap, aPos.xz / 50.0f)) - vec3(0.5f)) * 2.0f;
 	vs_out.normal = mat3(transpose(inverse(model))) * n;
}
