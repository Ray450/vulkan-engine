#version 450

#include "common.glsl"

layout(binding = 1) uniform sampler2D texSampler;


layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec4 fragColor;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec3 fragNormal;  // Normal vector from vertex shader


layout(location = 0) out vec4 outColor;


void main() {

    // NDC coords
    vec2 uv = gl_FragCoord.xy / ubo.iResolution.xy * 2.0 - 1.0;
    uv.xy = -uv.xy;
    float aspect = ubo.iResolution.x / ubo.iResolution.y;
    // uv.x *= aspect;

    uv = fragPos.xy;

    if (pushData.padding.x == 1.0) {
        outColor = pushData.color;
    } else if (pushData.padding.x == 2.0) {
        outColor = texture(texSampler, fragTexCoord);
    } else {
        outColor = fragColor;
    }


    if(outColor.a == 0.0) {
                discard;
    }
}
