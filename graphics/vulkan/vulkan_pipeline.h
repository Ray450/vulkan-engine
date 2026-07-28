#ifndef VULKAN_PIPELINE_H
#define VULKAN_PIPELINE_H

#include "vulkan_types.h"
#include "vulkan_utils.h"
#include "file_utils.h"

void vk_create_graphics_pipeline(VulkanContext* context);
void vk_create_compute_pipeline(
    VkDevice device,
    const char* filename,
    VkDescriptorSetLayout computeDescriptorSetLayout,
    VkPipelineLayout* outComputePipelineLayout,
    VkPipeline* outComputePipeline
);
void vk_create_graphics_pipeline(VulkanContext* context, VkDevice device,
    const char* vertShaderPath,
    const char* fragShaderPath,
    VkDescriptorSetLayout descriptorSetLayout,
    VkPipelineLayout* outPipelineLayout,
    VkPipeline* outGraphicsPipeline,
    VkRenderPass renderPass,
    int vertex_format,
    VkCullModeFlags cullMode,
    VkPrimitiveTopology topology,
    VkCompareOp depthOp);

void vk_bind_pipeline(GraphicsObject* object);

void vk_push_constants(GraphicsObject* object);

void vk_cleanup_pipeline(VkDevice device, VkPipelineLayout* pipelineLayout, VkPipeline* graphicsPipeline);

#endif // VULKAN_PIPELINE_H