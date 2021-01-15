#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 1, binding = 0) uniform MaterialData
{   
	vec4 albedo;
	float roughness;
	float metalness;
	float heightScale;
	float dummy1;
	float hasAlbedoTexture;
	float hasNormalTexture;
	float hasRoughnessTexture;
	float hasMetalnessTexture;
	float hasOcclusionTexture;
	float hasHeightTexture;
	float dummy2;
	float dummy3;
} material;

layout(location = 0) out vec4 outColor;

void main()
{
	outColor = material.albedo;
}