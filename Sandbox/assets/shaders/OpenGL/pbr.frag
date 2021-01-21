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

layout(binding = 0) uniform sampler2D albedoTextureSampler;
layout(binding = 1) uniform sampler2D normalTextureSampler;
layout(binding = 2) uniform sampler2D roughnessTextureSampler;
layout(binding = 3) uniform sampler2D metalnessTextureSampler;
layout(binding = 4) uniform sampler2D occlusionTextureSampler;
layout(binding = 5) uniform sampler2D heightTextureSampler;

layout(binding = 6) uniform samplerCube irradianceMap;
layout(binding = 7) uniform samplerCube prefilterMap;
layout(binding = 8) uniform sampler2D brdfLUT;

in vec2 fragTexCoords;
in vec3 fragPosition;
in vec3 fragNormal;
in vec3 cameraPosition;
in mat3 TBN;

out vec4 outColor;

const float PI = 3.14159265359;

float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 FresnelSchlick(float cosTheta, vec3 F0);
vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);
vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir);

void main()
{
	vec3 V = normalize(cameraPosition - fragPosition);

	vec2 texCoords;
	if(material.hasHeightTexture > 0.0f)
		texCoords = ParallaxMapping(fragTexCoords, transpose(TBN) * V);
	else
		texCoords = fragTexCoords;

	vec3 albedo;
	if(material.hasAlbedoTexture > 0.0f)
		albedo = texture(albedoTextureSampler, texCoords).rgb;
	else
		albedo = material.albedo.rgb;

	vec3 normal;
	if(material.hasNormalTexture > 0.0f)
		normal = normalize(TBN * normalize(texture(normalTextureSampler, texCoords).xyz * 2.0 - 1.0));
	else
		normal = normalize(fragNormal);

	float roughness;
	if(material.hasRoughnessTexture > 0.0f)
		roughness = texture(roughnessTextureSampler, texCoords).r;
	else
		roughness = material.roughness;

	float metalness;
	if(material.hasMetalnessTexture > 0.0f)
		metalness = texture(metalnessTextureSampler, texCoords).r;
	else
		metalness = material.metalness;

	float occlusion = 1.0;
	if(material.hasOcclusionTexture > 0.0f)
		occlusion = texture(occlusionTextureSampler, texCoords).r;

	vec3 F0 = vec3(0.04); 
	F0 = mix(F0, albedo, metalness);

	vec3 Lo = vec3(0.0);

	vec3 lightPositions[2];
	lightPositions[0] = vec3(-7.0, 0.0, 0.0);
	lightPositions[1] = vec3(7.0, 0.0, 0.0);

	vec3 lightColor[2];
	lightColor[0] = vec3(150.0, 100.0, 50.0);
	lightColor[1] = vec3(150.0, 100.0, 50.0);

	for(int i = 0; i < 2; ++i) 
	{
		// calculate per-light radiance
		vec3 L = normalize(lightPositions[i] - fragPosition);
		vec3 H = normalize(V + L);
		float distance = length(lightPositions[i] - fragPosition);
		float attenuation = 1.0 / (distance * distance);
		vec3 radiance = lightColor[i] * attenuation;

		// Cook-Torrance BRDF
		float NDF = DistributionGGX(normal, H, roughness);   
		float G = GeometrySmith(normal, V, L, roughness);      
		vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

		vec3 nominator = NDF * G * F; 
		float denominator = 4.0 * max(dot(normal, V), 0.0) * max(dot(normal, L), 0.0) + 0.001;
		vec3 specular = nominator / denominator;

		vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;
		kD *= 1.0 - metalness;	  

		float NdotL = max(dot(normal, L), 0.0);        

		Lo += (kD * albedo / PI + specular) * radiance * NdotL;
	}
	
	vec3 F = FresnelSchlickRoughness(max(dot(normal, V), 0.0), F0, roughness); 

	vec3 kS = F;
	vec3 kD = 1.0 - kS;

	vec3 irradiance = texture(irradianceMap, normal).rgb;
	vec3 diffuse = kD * irradiance * albedo;

	const float MAX_REFLECTION_LOD = 4.0;
	vec3 R = reflect(-V, normal);
    vec3 prefilteredColor = textureLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;
    vec2 brdf  = texture(brdfLUT, vec2(max(dot(normal, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

	vec3 ambient = (diffuse + specular)  * occlusion;
	vec3 color = ambient + Lo;

	// HDR tonemapping
	color = color / (color + vec3(1.0));
	// gamma correct
	color = pow(color, vec3(1.0/2.2)); 

	outColor = vec4(color, 1.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;

	float nom   = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r * r) / 8.0;

	float nom   = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
} 

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{ 
	// number of depth layers
	const float minLayers = 8.0;
	const float maxLayers = 32.0;
	float numLayers = mix(maxLayers, minLayers, max(dot(vec3(0.0, 0.0, 1.0), viewDir), 0.0));  
	// calculate the size of each layer
	float layerDepth = 1.0 / numLayers;
	// depth of current layer
	float currentLayerDepth = 0.0;
	// the amount to shift the texture coordinates per layer (from vector P)
	vec2 P = viewDir.xy * material.heightScale; 
	vec2 deltaTexCoords = P / numLayers;

	// get initial values
	vec2  currentTexCoords = texCoords;
	float currentDepthMapValue = -(texture(heightTextureSampler, currentTexCoords).r - 1.0);
  
	while(currentLayerDepth < currentDepthMapValue)
	{
		// shift texture coordinates along direction of P
		currentTexCoords -= deltaTexCoords;
		// get depthmap value at current texture coordinates
		currentDepthMapValue = -(texture(heightTextureSampler, currentTexCoords).r - 1.0);  
		// get depth of next layer
		currentLayerDepth += layerDepth;  
	}

	// get texture coordinates before collision (reverse operations)
	vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

	// get depth after and before collision for linear interpolation
	float afterDepth  = currentDepthMapValue - currentLayerDepth;
	float beforeDepth = -(texture(heightTextureSampler, prevTexCoords).r - 1.0) - currentLayerDepth + layerDepth;
 
	// interpolation of texture coordinates
	float t = afterDepth / (afterDepth - beforeDepth);
	vec2 finalTexCoords = (1.0 - t) * currentTexCoords + t * prevTexCoords;

	return finalTexCoords;  
} 