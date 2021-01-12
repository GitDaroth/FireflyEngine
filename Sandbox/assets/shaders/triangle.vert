#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform SceneData
{   
    mat4 viewMatrix;
    mat4 projectionMatrix;
    mat4 viewProjectionMatrix;
    vec3 cameraPosition;
} scene;

layout(set = 1, binding = 0) uniform MaterialData
{   
    vec4 color;
} material;

layout(set = 2, binding = 0) uniform ObjectData
{   
    mat4 modelMatrix;
    mat3 normalMatrix;
} object;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec3 inBitangent;
layout(location = 4) in vec2 inTexCoords;

layout(location = 0) out vec4 fragColor;

void main() 
{
    gl_Position = scene.viewProjectionMatrix * object.modelMatrix * vec4(inPosition, 1.0);
    fragColor = material.color;
}