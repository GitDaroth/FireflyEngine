#version 450

struct SceneData
{
    mat4 viewMatrix;
    mat4 projectionMatrix;
    mat4 viewProjectionMatrix;
    vec4 cameraPosition;
};

struct ObjectData
{
    mat4 modelMatrix;
    mat4 normalMatrix;
};

uniform SceneData scene;
uniform ObjectData object;

in vec3 inPosition;
in vec3 inNormal;
in vec3 inTangent;
in vec3 inBitangent;
in vec2 inTexCoords;

out vec2 fragTexCoords;
out vec3 fragPosition;
out vec3 fragNormal;
out vec3 cameraPosition;
out mat3 TBN;

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