#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform CameraData
{   
    vec4 position;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    mat4 viewProjectionMatrix;
} camera;

layout(push_constant) uniform EntityData
{
    mat4 modelMatrix;
} entity;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec3 inBitangent;
layout(location = 4) in vec2 inTexCoords;

layout(location = 0) out vec3 fragColor;

void main() 
{
    gl_Position = camera.viewProjectionMatrix * entity.modelMatrix * vec4(inPosition, 1.0);
    fragColor = inNormal;
}