#version 450

layout(set = 1, binding = 0) uniform sampler2D hdrTexture;

layout(location = 0) in vec3 worldPos;
layout(location = 0) out vec4 fragColor;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main()
{		
    vec2 uv = SampleSphericalMap(normalize(worldPos));
    vec3 color = texture(hdrTexture, uv).rgb;
    
    fragColor = vec4(color, 1.0);
}