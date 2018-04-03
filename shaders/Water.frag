#version 330 core

in VS_OUT {
    vec4 fragPos;
    vec3 normal;
} fs_in;

out vec4 fragColor;

uniform vec3 viewPos;
uniform vec3 lightDir;

uniform vec3 deepWaterColor;
uniform vec3 shallowWaterColor;
// Wave normals described in texture
uniform int waveMapCount;
uniform sampler2D waveMaps[5];

uniform samplerCube skybox;

void main()
{
    vec3 n = normalize(fs_in.normal);
    vec3 eyeVec = normalize(viewPos - vec3(fs_in.fragPos));
    vec3 halfwayDir = normalize(lightDir + eyeVec);
    vec3 reflectVec = normalize(2 * dot(eyeVec, n) * n - eyeVec);

    // Incident angle, reflection angle and transmission(refraction) angle
    float thetaI = acos(dot(eyeVec, n));
    float thetaR = acos(dot(reflectVec, n));
    float thetaT = asin(0.75 * sin(thetaI));

    // The reflectivity factor, 1-reflectivity is the refraction factor
    float reflectivity;
    if (abs(thetaI) >= 0.000001) {
        float t1 = sin(thetaT - thetaI), t2 = sin(thetaT + thetaI);
        float t3 = tan(thetaT - thetaI), t4 = tan(thetaT + thetaI);
        reflectivity = 0.5 * (t1*t1/(t2*t2) + t3*t3/(t4*t4));
        if (reflectivity > 1.0) reflectivity = 1.0;
    } else {
        reflectivity = 0;
    }

    // Reflection color component
    vec4 r = texture(skybox, reflectVec);

    // Transmission color component
    vec4 t = vec4(deepWaterColor, 1.0);

    // Calculate Fresnel Reflection and Refraction
    fragColor = reflectivity * r + (1 - reflectivity) * t;
}