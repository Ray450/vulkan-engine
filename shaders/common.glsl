layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec2 iResolution;
    float iTime;
    float padding;
} ubo;

struct InstanceData {
    vec4  instanceOffsets;
};

layout(binding = 2) readonly buffer InstanceBuffer {
    // vec4 instanceOffsets[];
    InstanceData instances[];
};


layout(push_constant) uniform PushConstants {
    vec4 offset;
    vec4 color;
    vec4 shape;
    vec4 padding;
} pushData;
