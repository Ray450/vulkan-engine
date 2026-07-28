#version 450
#include "common.glsl"
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec4 fragColor;
layout(location = 2) out vec2 fragTexCoord;
layout(location = 3) out vec3 fragNormal;

void main()
{
    // Strip translation from view so skybox stays at infinity
    mat4 viewNoTranslation = mat4(mat3(ubo.view));

    vec4 pos = ubo.proj * viewNoTranslation * vec4(inPosition, 1.0);

    // xyww trick: depth always resolves to 1.0
    gl_Position = pos.xyww;

    fragPos = inPosition;
}
