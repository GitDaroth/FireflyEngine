#type vertex
	#version 330 core
			
	layout(location = 0) in vec3 position;
	layout(location = 1) in vec3 normal;
	layout(location = 2) in vec2 texCoords;

	uniform mat4 u_viewProjectionMat;
	uniform mat4 u_modelMat;

	out vec2 v_texCoords;
	out vec3 v_worldPosition;
	out vec3 v_worldNormal;

	void main()
	{
		v_texCoords = texCoords;
		v_worldPosition = (u_modelMat * vec4(position, 1.0)).xyz;
		v_worldNormal = mat3(u_modelMat) * normal;
		gl_Position = u_viewProjectionMat * vec4(v_worldPosition, 1.0);
	}

#type fragment
	#version 330 core
			
	out vec4 f_color;

	in vec2 v_texCoords;
	in vec3 v_worldPosition;
	in vec3 v_worldNormal;

	uniform vec3 u_cameraPosition;
	uniform vec4 u_albedoColor;
	uniform float u_roughness;
	uniform float u_metalness;
	uniform int u_hasAlbedoTexture;
	uniform int u_hasNormalTexture;
	uniform int u_hasRoughnessTexture;
	uniform int u_hasMetalnessTexture;
	uniform int u_hasOcclusionTexture;
	uniform int u_hasHeightTexture;
	uniform sampler2D u_albedoTexture;
	uniform sampler2D u_normalTexture;
	uniform sampler2D u_roughnessTexture;
	uniform sampler2D u_metalnessTexture;
	uniform sampler2D u_occlusionTexture;
	uniform sampler2D u_heightTexture;

	const float PI = 3.14159265359;

	vec3 sampleNormals(vec3 normal, vec2 texCoords, vec3 worldPosition)
	{
		vec3 tangentNormal = texture(u_normalTexture, texCoords).xyz * 2.0 - 1.0;

		vec3 Q1  = dFdx(worldPosition);
		vec3 Q2  = dFdy(worldPosition);
		vec2 st1 = dFdx(texCoords);
		vec2 st2 = dFdy(texCoords);

		vec3 N   = normal;
		vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
		vec3 B  = -normalize(cross(N, T));
		mat3 TBN = mat3(T, B, N);

		return normalize(TBN * tangentNormal);
	}

	float DistributionGGX(vec3 N, vec3 H, float roughness)
	{
		float a = roughness*roughness;
		float a2 = a*a;
		float NdotH = max(dot(N, H), 0.0);
		float NdotH2 = NdotH*NdotH;

		float nom   = a2;
		float denom = (NdotH2 * (a2 - 1.0) + 1.0);
		denom = PI * denom * denom;

		return nom / denom;
	}

	float GeometrySchlickGGX(float NdotV, float roughness)
	{
		float r = (roughness + 1.0);
		float k = (r*r) / 8.0;

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
		if(u_hasAlbedoTexture != 0)
			albedo = pow(texture(u_albedoTexture, v_texCoords).rgb, vec3(2.2));
		else
			albedo = u_albedoColor.rgb;

		vec3 normal = normalize(v_worldNormal);
//		if(u_hasNormalTexture != 0)
//			normal = sampleNormals(normal, v_texCoords, v_worldPosition);

		float roughness;
		if(u_hasRoughnessTexture != 0)
			roughness = texture(u_roughnessTexture, v_texCoords).r;
		else
			roughness = u_roughness;

		float metalness;
		if(u_hasMetalnessTexture != 0)
			metalness = texture(u_metalnessTexture, v_texCoords).r;
		else
			metalness = u_metalness;

		float occlusion = 1.0;
		if(u_hasOcclusionTexture != 0)
			occlusion = texture(u_occlusionTexture, v_texCoords).r;

		vec3 viewDirection = normalize(u_cameraPosition - v_worldPosition);

		vec3 F0 = vec3(0.04); 
		F0 = mix(F0, albedo, metalness);

		// reflectance equation
		vec3 Lo = vec3(0.0);


		vec3 lightPosition[2];
		lightPosition[0] = vec3(7.0, -1.0, 0.0);
		lightPosition[1] = vec3(-7.0, -1.0, 0.0);

		vec3 lightColor[2];
		lightColor[0] = vec3(150.0, 100.0, 50.0);
		lightColor[1] = vec3(50.0, 150.0, 100.0);

		for(int i = 0; i < 2; ++i) 
		{
			// calculate per-light radiance
			vec3 L = normalize(lightPosition[i] - v_worldPosition);
			vec3 H = normalize(viewDirection + L);
			float distance = length(lightPosition[i] - v_worldPosition);
			float attenuation = 1.0 / (distance * distance);
			vec3 radiance = lightColor[i] * attenuation;

			// Cook-Torrance BRDF
			float NDF = DistributionGGX(normal, H, roughness);   
			float G = GeometrySmith(normal, viewDirection, L, roughness);      
			vec3 F = fresnelSchlick(max(dot(H, viewDirection), 0.0), F0);
           
			vec3 nominator = NDF * G * F; 
			float denominator = 4.0 * max(dot(normal, viewDirection), 0.0) * max(dot(normal, L), 0.0) + 0.001; // 0.001 to prevent divide by zero.
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