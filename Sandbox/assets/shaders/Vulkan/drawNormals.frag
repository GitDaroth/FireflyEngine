#version 450

layout(set = 1, binding = 0) uniform MaterialData
{
    vec4 albedo;
    float roughness;
    float metalness;
    float heightScale;
    float hasAlbedoTexture;
    float hasNormalTexture;
    float hasRoughnessTexture;
    float hasMetalnessTexture;
    float hasOcclusionTexture;
    float hasHeightTexture;
} material;

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = material.albedo;
}