#ifndef VULKAN_DESCRIPTOR_H
#define VULKAN_DESCRIPTOR_H

#include "vulkan_types.h"
#include "vulkan_utils.h"

void vk_create_graphics_descriptor_set_layout(VkDevice device, VkDescriptorSetLayout* descriptorSetLayout);
void vk_create_compute_descriptor_set_layout(VkDevice device, VkDescriptorSetLayout* computeDescriptorSetLayout);
void vk_create_descriptor_set_layouts(VkDevice device, VkDescriptorSetLayout* graphicsDescriptorSetLayout, VkDescriptorSetLayout* computeDescriptorSetLayout);


void vk_create_graphics_descriptor_sets(
    VkDevice device,
    const VkBuffer* uniformBuffers,  // Pass by reference to avoid copying
    const VkDescriptorSetLayout descriptorSetLayout, // Pass descriptor set layout as reference
    VkDescriptorPool descriptorPool,
    const VkImageView textureImageView,  // Pass by reference for texture image view
    const VkSampler textureSampler,     // Pass by reference for texture sampler
    VkDescriptorSet* descriptorSets,  // Pass by reference to modify the descriptor sets
    VkBuffer instanceBuffer,      
    VkDeviceSize instanceBufferSize  

);

void vk_create_compute_descriptor_sets(
    VulkanContext* context,
    VkDevice device,
    VkBuffer* buffers,      // flat array of all buffers [binding0, binding1, ..., bindingN]
    uint32_t bindingCount,  // how many bindings this shader uses
    VkDescriptorPool descriptorPool,
    VkDescriptorSet* descriptorSets,
    size_t setCount
);

void vk_create_graphics_descriptor_pool(VkDevice device, VkDescriptorPool* descriptorPool);
void vk_create_compute_descriptor_pool(VkDevice device, VkDescriptorPool* descriptorPool);

void vk_bind_descriptor_sets(GraphicsObject* object);

#endif // VULKAN_DESCRIPTOR_H