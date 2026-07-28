#ifndef VULKAN_UTILS_H
#define VULKAN_UTILS_H

#include <vulkan/vulkan.h>
#include "vulkan_types.h"

// Memory and buffer utilities
uint32_t vk_find_memory_type(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
void vk_destroy_debug_utils_messenger_ext(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

// Image utilities
void vk_create_image(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage* image, VkDeviceMemory* imageMemory, VkImageViewType viewType);
void vk_transition_image_layout(VulkanContext* context, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageViewType viewType);
void vk_copy_buffer_to_image(VulkanContext* context, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
VkImageView vk_create_image_view(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageViewType viewType);

// Device and format utilities

QueueFamilyIndices vk_find_queue_families(VkSurfaceKHR surface, VkPhysicalDevice device);
bool vk_check_device_extension_support(VkPhysicalDevice device, const char* deviceExtensions[], size_t deviceExtensionCount);
SwapChainSupportDetails vk_query_swap_chain_support(VkSurfaceKHR surface, VkPhysicalDevice device);
VkFormat vk_find_supported_format(VkPhysicalDevice physicalDevice, VkFormat* candidates, size_t candidateCount, VkImageTiling tiling, VkFormatFeatureFlags features);
VkFormat vk_find_depth_format(VkPhysicalDevice physicalDevice);
bool vk_has_stencil_component(VkFormat format);

// Command utilities
VkCommandBuffer vk_begin_single_time_commands(VulkanContext* context);
void vk_end_single_time_commands(VulkanContext* context, VkCommandBuffer commandBuffer);
VkShaderModule vk_create_shader_module(VkDevice device, const char* code, size_t codeSize);

#endif // VULKAN_UTILS_H