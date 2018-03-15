#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 vNormal;
out vec4 vFragPosition;

void main() {
    gl_Position = model * vec4(aPos, 1.0);
	vNormal = normalize(mat3(transpose(inverse(model))) * aNormal);
 	vFragPosition = model * vec4(aPos, 1.0);
}
