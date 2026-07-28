#version 450

#include "common.glsl"

//layout(location = 0) in vec2 inPosition;
layout(location = 0) in vec3 inPosition;

layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec4 fragPos;    // Output position to fragment shader

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);


    fragColor = inColor;
    fragPos = vec4(inPosition, 1.0);

}