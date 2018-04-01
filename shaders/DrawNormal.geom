#version 330 core

layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

in vec3 vNormal[3];

uniform mat4 view;
uniform mat4 projection;

void generateLine(int i)
{
    gl_Position = projection * view * (gl_in[i].gl_Position);
    EmitVertex();
    gl_Position = projection * view * (gl_in[i].gl_Position + 0.25 * vec4(vNormal[i], 0.0));
    EmitVertex();
    EndPrimitive();
}

void main()
{
    generateLine(0);
    generateLine(1);
    generateLine(2);
}