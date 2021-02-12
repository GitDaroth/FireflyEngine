#version 450

layout(set = 1, binding = 0) uniform samplerCube environmentMap;

layout(location = 0) in vec3 worldPos;
layout(location = 0) out vec4 fragColor;

void main()
{
    vec3 envColor = texture(environmentMap, normalize(worldPos)).rgb;

    // HDR tonemap and gamma correct
    envColor = envColor / (envColor + vec3(1.0));
    envColor = pow(envColor, vec3(1.0 / 2.2)); 

    fragColor = vec4(envColor, 1.0);
}