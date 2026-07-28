#ifndef VULKAN_COMMAND_H
#define VULKAN_COMMAND_H

#include "vulkan_types.h"
#include "vulkan_utils.h"

void vk_create_command_pool(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkCommandPool* commandPool);
void vk_create_command_buffers(VkDevice device, VkCommandPool commandPool, VkCommandBuffer* commandBuffers, uint32_t maxFramesInFlight);
void vk_create_compute_command_buffers(VkDevice device, VkCommandPool commandPool, VkCommandBuffer* commandBuffers, uint32_t maxFramesInFlight);
void vk_set_viewport(float x, float y, float width, float height, float minDepth, float maxDepth);
void vk_draw(GraphicsObject* object, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);


#endif // VULKAN_COMMAND_H