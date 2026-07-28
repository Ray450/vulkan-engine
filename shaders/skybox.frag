#version 450
#include "common.glsl"
layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec4 fragColor;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec3 fragNormal;  // Normal vector from vertex shader


layout(binding = 1) uniform samplerCube texSampler;

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = texture(texSampler, fragPos);
}
