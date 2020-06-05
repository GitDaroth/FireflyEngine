#type vertex
	#version 330 core
			
	layout(location = 0) in vec3 position;
	layout(location = 1) in vec3 normal;
	layout(location = 2) in vec2 texCoords;

	uniform mat4 viewProjectionMat;
	uniform mat4 modelMat;

	out vec2 v_texCoord;

	void main()
	{
		v_texCoord = texCoords;
		gl_Position = viewProjectionMat * modelMat * vec4(position, 1.0);
	}

#type fragment
	#version 330 core
			
	layout(location = 0) out vec4 color;

	in vec2 v_texCoord;

	uniform vec4 diffuseColor;
	uniform int hasDiffuseTexture;
	uniform sampler2D diffuseTexture;

	void main()
	{
		if(hasDiffuseTexture != 0)
			color = texture(diffuseTexture, v_texCoord);
		else
			color = diffuseColor;
	}