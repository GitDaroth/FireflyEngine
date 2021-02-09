#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec3 inBitangent;
layout(location = 4) in vec2 inTexCoords;

layout(location = 0) out vec2 texCoords;

void main()
{
    gl_Position = vec4(inPosition.xy, 0.0, 1.0); 
    texCoords = inTexCoords;

    // Adjust for Vulkan texture and clip space coordinate systems
    texCoords.y = 1.0 - texCoords.y;
    gl_Position.y = -gl_Position.y;
}