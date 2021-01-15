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

out vec3 geomNormal;
out mat4 mvp;

void main() 
{
    gl_Position = vec4(inPosition, 1.0);
    geomNormal = inNormal;
    mvp = scene.viewProjectionMatrix * object.modelMatrix;
}