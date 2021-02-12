#version 450

layout(location = 0) out vec4 FragColor;

layout(location = 0) in vec2 TexCoords;

layout(set = 0, binding = 0) uniform sampler2D screenTextureSampler;

void main()
{
    FragColor = texture(screenTextureSampler, TexCoords);
}