#version 330 core

layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform sampler2D heightMap;
uniform sampler2D normalMap;

out vec3 vNormal;
out vec4 vFragPosition;

void main() {
    vec3 height = (vec3(texture(heightMap, aPos.xz / 64.0f)) - vec3(0.5f)) * 5.0f;
    vec3 pos = aPos + height;
    gl_Position = model * vec4(pos, 1.0);
    vFragPosition = gl_Position;

    vec3 n = (vec3(texture(normalMap, aPos.xz / 64.0f)) - vec3(0.5f)) * 2.0f;
	vNormal = mat3(transpose(inverse(model))) * n;
}
