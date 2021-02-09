#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec3 inBitangent;
layout(location = 4) in vec2 inTexCoords;

layout(set = 0, binding = 0) uniform SceneData
{
	mat4 viewMatrix;
	mat4 projectionMatrix;
	mat4 viewProjectionMatrix;
	vec4 cameraPosition;
} scene;

layout(location = 0) out vec3 worldPos;

void main()
{
	worldPos = inPosition;

	mat4 rotView = mat4(mat3(scene.viewMatrix));
	vec4 clipPos = scene.projectionMatrix * rotView * vec4(worldPos, 1.0);

	gl_Position = clipPos.xyww;

	// Adjust for Vulkan texture and clip space coordinate systems
	gl_Position.y = -gl_Position.y;
}