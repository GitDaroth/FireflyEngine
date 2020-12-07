#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UboPerFrame
{
    mat4 viewMatrix;
    mat4 projectionMatrix;
} uboPerFrame;

layout(binding = 1) uniform UboPerObject
{
    mat4 modelMatrix;
} uboPerObject;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec3 inBitangent;
layout(location = 4) in vec3 inTexCoords;

layout(location = 0) out vec3 fragColor;

void main() 
{
    gl_Position = uboPerFrame.projectionMatrix * uboPerFrame.viewMatrix * uboPerObject.modelMatrix * vec4(inPosition, 1.0);
    fragColor = inNormal;
}