#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec3 inBitangent;
layout(location = 4) in vec2 inTexCoords;

layout(set = 0, binding = 0) uniform CameraData
{
    mat4 projectionMatrix;
    mat4 viewMatrix;
} camera;

layout(location = 0) out vec3 worldPos;

void main()
{
    worldPos = inPosition;  
    gl_Position =  camera.projectionMatrix * camera.viewMatrix * vec4(worldPos, 1.0);

    // Adjust for Vulkan texture and clip space coordinate systems
    gl_Position.y = -gl_Position.y;
}