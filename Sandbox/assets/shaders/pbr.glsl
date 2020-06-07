#type vertex
	#version 330 core
			
	layout(location = 0) in vec3 a_position;
	layout(location = 1) in vec3 a_normal;
	layout(location = 2) in vec3 a_tangent;
	layout(location = 3) in vec3 a_bitangent;
	layout(location = 4) in vec2 a_texCoords;

	uniform mat4 u_viewProjectionMat;
	uniform mat4 u_modelMat;
	uniform mat3 u_normalMat;
	uniform vec3 u_cameraPosition;

	out vec2 v_texCoords;
	out vec3 v_position;
	out vec3 v_normal;
	out vec3 v_cameraPosition;
	out vec3 v_lightPosition[2];

	void main()
	{	
		vec3 N = normalize(u_normalMat * a_normal); 
		vec3 T = normalize(u_normalMat * a_tangent);
		vec3 B = normalize(u_normalMat * a_bitangent);
		mat3 invTBN = transpose(mat3(T, B, N));

		v_texCoords = a_texCoords;
		v_normal = invTBN * N;
		vec3 worldPosition = (u_modelMat * vec4(a_position, 1.0)).xyz;
		v_position = invTBN * worldPosition;
		v_cameraPosition = invTBN * u_cameraPosition;
		v_lightPosition[0] = invTBN * vec3(7.0, -1.0, 0.0);
		v_lightPosition[1] = invTBN * vec3(-7.0, -1.0, 0.0);

		gl_Position = u_viewProjectionMat * vec4(worldPosition, 1.0);
	}

#type fragment
	#version 330 core
			
	out vec4 f_color;

	in vec2 v_texCoords;
	in vec3 v_position;
	in vec3 v_normal;
	in vec3 v_cameraPosition;
	in vec3 v_lightPosition[2];

	uniform vec4 u_albedoColor;
	uniform float u_roughness;
	uniform float u_metalness;
	uniform int u_useAlbedoMap;
	uniform int u_useNormalMap;
	uniform int u_useRoughnessMap;
	uniform int u_useMetalnessMap;
	uniform int u_useOcclusionMap;
	uniform int u_useHeightMap;
	uniform sampler2D u_albedoMap;
	uniform sampler2D u_normalMap;
	uniform sampler2D u_roughnessMap;
	uniform sampler2D u_metalnessMap;
	uniform sampler2D u_occlusionMap;
	uniform sampler2D u_heightMap;

	const float PI = 3.14159265359;

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

	vec3 fresnelSchlick(float cosTheta, vec3 F0)
	{
		return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
	}

	void main()
	{
		vec3 albedo;
		if(u_useAlbedoMap != 0)
			albedo = pow(texture(u_albedoMap, v_texCoords).rgb, vec3(2.2));
		else
			albedo = u_albedoColor.rgb;

		vec3 normal;
		if(u_useNormalMap != 0)
			normal = normalize(texture(u_normalMap, v_texCoords).xyz * 2.0 - 1.0);
		else
			normal = normalize(v_normal);

		float roughness;
		if(u_useRoughnessMap != 0)
			roughness = texture(u_roughnessMap, v_texCoords).r;
		else
			roughness = u_roughness;

		float metalness;
		if(u_useMetalnessMap != 0)
			metalness = texture(u_metalnessMap, v_texCoords).r;
		else
			metalness = u_metalness;

		float occlusion = 1.0;
		if(u_useOcclusionMap != 0)
			occlusion = texture(u_occlusionMap, v_texCoords).r;

		vec3 V = normalize(v_cameraPosition - v_position);

		vec3 F0 = vec3(0.04); 
		F0 = mix(F0, albedo, metalness);

		// reflectance equation
		vec3 Lo = vec3(0.0);



		vec3 lightColor[2];
		lightColor[0] = vec3(150.0, 100.0, 50.0);
		lightColor[1] = vec3(50.0, 150.0, 100.0);

		for(int i = 0; i < 2; ++i) 
		{
			// calculate per-light radiance
			vec3 L = normalize(v_lightPosition[i] - v_position);
			vec3 H = normalize(V + L);
			float distance = length(v_lightPosition[i] - v_position);
			float attenuation = 1.0 / (distance * distance);
			vec3 radiance = lightColor[i] * attenuation;

			// Cook-Torrance BRDF
			float NDF = DistributionGGX(normal, H, roughness);   
			float G = GeometrySmith(normal, V, L, roughness);      
			vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
           
			vec3 nominator = NDF * G * F; 
			float denominator = 4.0 * max(dot(normal, V), 0.0) * max(dot(normal, L), 0.0) + 0.001; // 0.001 to prevent divide by zero.
			vec3 specular = nominator / denominator;
        
			// kS is equal to Fresnel
			vec3 kS = F;
			// for energy conservation, the diffuse and specular light can't
			// be above 1.0 (unless the surface emits light); to preserve this
			// relationship the diffuse component (kD) should equal 1.0 - kS.
			vec3 kD = vec3(1.0) - kS;
			// multiply kD by the inverse metalness such that only non-metals 
			// have diffuse lighting, or a linear blend if partly metal (pure metals
			// have no diffuse light).
			kD *= 1.0 - metalness;	  

			// scale light by NdotL
			float NdotL = max(dot(normal, L), 0.0);        

			// add to outgoing radiance Lo
			Lo += (kD * albedo / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
		}

		// ambient lighting (note that the next IBL tutorial will replace 
		// this ambient lighting with environment lighting).
		vec3 ambient = vec3(0.03) * albedo * occlusion;
    
		vec3 color = ambient + Lo;

		// HDR tonemapping
		color = color / (color + vec3(1.0));
		// gamma correct
		color = pow(color, vec3(1.0/2.2)); 

		f_color = vec4(color, 1.0);
	}