#version 330 core

in VS_OUT {
    vec4 fragPos;
    vec3 normal;
    vec2 texCoord;
} fs_in;

out vec4 fragColor;

uniform vec3 viewPos;
uniform vec3 lightDir;
uniform vec3 lightPos;

uniform vec3 diffuse;
uniform vec3 ambient;
uniform vec3 specular;

uniform sampler2D normalMap;
uniform sampler2D heightMap;
uniform samplerCube skybox;

void main()
{
    vec3 n = normalize(2.0f * vec3(texture(normalMap, fs_in.texCoord)) - 1.0f);
    vec3 eyeVec = normalize(viewPos - vec3(fs_in.fragPos));
    vec3 halfwayDir = normalize(lightDir + eyeVec);

    vec4 c1 = vec4(ambient, 1.0) * texture(skybox, lightPos - fs_in.fragPos.xyz);
    vec4 c2 = vec4(specular, 1.0) * texture(skybox, lightPos - fs_in.fragPos.xyz);


    fragColor = vec4(0);
    fragColor += vec4(diffuse, 1.0);
    fragColor += c1 * max(dot(normalize(lightDir), n), 0);
    fragColor += c2 * pow(max(dot(n, halfwayDir), 0.0), 8.0f);
}