#ifndef VULKAN_RENDERPASS_H
#define VULKAN_RENDERPASS_H

#include "vulkan_types.h"
#include "vulkan_utils.h"

void vk_create_render_pass(VkDevice device, VkPhysicalDevice physicalDevice, VkFormat swapChainImageFormat, VkRenderPass* renderPass);
void vk_create_depth_resources(VkDevice device, VkPhysicalDevice physicalDevice, VkExtent2D swapChainExtent, VkImage* depthImage, VkDeviceMemory* depthImageMemory, VkImageView* depthImageView);
void vk_create_framebuffers(VkDevice device, VkRenderPass renderPass, VkImageView* swapChainImageViews, VkImageView depthImageView, VkFramebuffer* swapChainFramebuffers, VkExtent2D swapChainExtent, uint32_t imageCount);


#endif // VULKAN_RENDERPASS_H
