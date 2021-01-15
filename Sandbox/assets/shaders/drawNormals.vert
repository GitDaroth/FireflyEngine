#version 450
#extension GL_ARB_separate_shader_objects : enable

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

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec4 inNormal;
layout(location = 2) in vec4 inTangent;
layout(location = 3) in vec4 inBitangent;
layout(location = 4) in vec2 inTexCoords;

layout(location = 0) out vec3 geomNormal;
layout(location = 1) out mat4 mvp;

void main() 
{
    gl_Position = inPosition;
    geomNormal = inNormal.xyz;
    mvp = scene.viewProjectionMatrix * object.modelMatrix;
}