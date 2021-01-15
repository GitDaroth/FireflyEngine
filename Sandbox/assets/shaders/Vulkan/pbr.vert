#version 450

layout(set = 0, binding = 0) uniform SceneData
{   
    mat4 viewMatrix;
    mat4 projectionMatrix;
    mat4 viewProjectionMatrix;
    vec4 cameraPosition;
} scene;

layout(set = 3, binding = 0) uniform ObjectData
{   
    mat4 modelMatrix;
    mat4 normalMatrix;
} object;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec3 inBitangent;
layout(location = 4) in vec2 inTexCoords;

layout(location = 0) out vec2 fragTexCoords;
layout(location = 1) out vec3 fragPosition;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec3 cameraPosition;
layout(location = 4) out mat3 TBN;

void main() 
{
    vec3 worldPosition = (object.modelMatrix * vec4(inPosition, 1.0)).xyz;
    gl_Position = scene.viewProjectionMatrix * vec4(worldPosition, 1.0);

    vec3 N = normalize(mat3(object.normalMatrix) * inNormal); 
	vec3 T = normalize(mat3(object.normalMatrix) * inTangent);
	vec3 B = normalize(mat3(object.normalMatrix) * inBitangent);
	TBN = mat3(T, B, N);

    fragTexCoords = inTexCoords;
    fragNormal = N;
	fragPosition = worldPosition;
	cameraPosition = scene.cameraPosition.xyz;
}