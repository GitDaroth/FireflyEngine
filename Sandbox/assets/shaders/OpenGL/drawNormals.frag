#version 450

struct MaterialData
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
};

uniform MaterialData material;

out vec4 outColor;

void main()
{
    outColor = material.albedo;
}