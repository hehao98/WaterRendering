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
    vec3 reflectVec = 2 * dot(eyeVec, n) * n - eyeVec;

    // Incident angle, reflection angle and transmission(refraction) angle
    float thetaI = acos(dot(eyeVec, n));
    float thetaR = acos(dot(reflectVec, n));
    float thetaT = asin(0.75 * sin(thetaI));
    // The reflectivity factor, 1-reflectivity is the refraction factor
    float reflectivity;
    if (abs(thetaI) >= 0.000001) {
        float t1 = sin(thetaT - thetaI), t2 = sin(thetaT + thetaI);
        float t3 = tan(thetaT - thetaI), t4 = tan(thetaT + thetaI);
        reflectivity = clamp(0.5 * (t1*t1/(t2*t2) + t3*t3/(t4*t4)), 0.0, 1.0);
    } else {
        reflectivity = 0;
    }

    // Reflection color component
    vec4 r = texture(skybox, reflectVec);

    // Transmission color component
    vec4 t = vec4(diffuse, 1.0);

    // Calculate Fresnel Reflection and Refraction
    fragColor = reflectivity * r + (1 - reflectivity) * t;

    float dist = length(viewPos - vec3(fs_in.fragPos));
    vec4 fogColor = texture(skybox, vec3(-eyeVec.x, 0.0, -eyeVec.z));
    float fogFactor = 1 - exp(-0.003 * dist);
    fragColor = mix(fragColor, fogColor, fogFactor);
}