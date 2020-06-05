#type vertex
	#version 330 core
			
	layout(location = 0) in vec3 position;
	layout(location = 1) in vec2 texCoord;

	uniform mat4 viewProjectionMat;
	uniform mat4 modelMat;

	out vec2 v_texCoord;

	void main()
	{
		v_texCoord = texCoord;
		gl_Position = viewProjectionMat * modelMat * vec4(position, 1.0);
	}

#type fragment
	#version 330 core
			
	layout(location = 0) out vec4 color;

	in vec2 v_texCoord;

	uniform sampler2D textureSampler;

	void main()
	{
		color = texture(textureSampler, v_texCoord);
	}