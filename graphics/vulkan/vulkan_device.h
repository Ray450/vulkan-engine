#ifndef VULKAN_DEVICE_H
#define VULKAN_DEVICE_H
#include "vulkan_types.h"
void vk_pick_physical_device(VkPhysicalDevice* physicalDevice, VkPhysicalDeviceLimits* deviceLimits, VkInstance instance, VkSurfaceKHR surface, const char* deviceExtensions[], size_t deviceExtensionCount, bool enableValidationLayers);
void vk_create_logical_device(VkDevice* device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkQueue* graphicsQueue, VkQueue* presentQueue, VkQueue* computeQueue, const char* deviceExtensions[], size_t deviceExtensionCount, const char* validationLayers[], size_t validationLayerCount, bool enableValidationLayers);
void vk_print_device_limits(VkPhysicalDevice physicalDevice);

#endif // VULKAN_DEVICE_H