#version 450

layout(set = 0, binding = 0) uniform SceneData
{
    mat4 viewMatrix;
    mat4 projectionMatrix;
    mat4 viewProjectionMatrix;
    vec4 cameraPosition;
} scene;

layout(set = 3, binding = 0) uniform ObjectData
{
    mat4 modelMatrix;
    mat4 normalMatrix;
} object;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec3 inBitangent;
layout(location = 4) in vec2 inTexCoords;

layout(location = 0) out vec3 geomNormal;
layout(location = 1) out mat4 mvp;

void main() 
{
    gl_Position = vec4(inPosition, 1.0);
    geomNormal = inNormal;
    mvp = scene.viewProjectionMatrix * object.modelMatrix;
}