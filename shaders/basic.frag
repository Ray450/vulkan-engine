#version 450
#include "common.glsl"

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec4 fragPos;   // Fragment position input (3D space)

layout(location = 0) out vec4 outColor;

void main() {

    if (pushData.padding.x == 1.0) {
        outColor = pushData.color;
    } else {
        outColor = vec4(fragColor, 1.0);
    }

    if(outColor.a == 0.0) {
                discard;
        }
}
