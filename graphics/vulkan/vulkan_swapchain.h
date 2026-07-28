#ifndef VULKAN_SWAPCHAIN_H
#define VULKAN_SWAPCHAIN_H

#include "vulkan_types.h"
#include "vulkan_utils.h"

void vk_create_swap_chain(VkSwapchainKHR* swapChain, VkImage* swapChainImages, VkImageView* swapChainImageViews, VkFormat* swapChainImageFormat, VkExtent2D* swapChainExtent, VkFramebuffer* swapChainFramebuffers, size_t maxImageCount, size_t* actualImageCount, VkSurfaceKHR surface, VkPhysicalDevice physicalDevice, VkDevice device, VkRenderPass renderPass, GLFWwindow* window);

void vk_create_image_views(VkDevice device, VkImage* swapChainImages, VkImageView* swapChainImageViews, VkFormat swapChainImageFormat, size_t imageCount);

uint32_t vk_acquire_next_image();

int vk_get_width();
int vk_get_height();

void vk_present_image(uint32_t imageIndex);

void vk_recreate_swapChain(VkDevice device, VkSurfaceKHR surface, VkSwapchainKHR* swapChain, VkExtent2D* swapChainExtent, VkFormat* swapChainImageFormat, VkImage* swapChainImages, uint32_t* imageCount);
void vk_cleanup_swapChain(VkDevice device, VkSwapchainKHR swapChain, VkImage* swapChainImages, uint32_t imageCount);
void vk_recreate_swapChain();
void vk_cleanup_swap_chain();

#endif // VULKAN_SWAPCHAIN_H
