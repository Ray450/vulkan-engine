#ifndef VULKAN_IMAGE_H
#define VULKAN_IMAGE_H

#include "vulkan_types.h"
#include "vulkan_utils.h"

#include "vulkan_buffer.h"

bool vk_create_texture_from_pixels(VulkanContext* context, Texture* texture,
                             unsigned char* pixels, int width, int height,
                             VkFormat format, VkImageViewType viewType);

bool vk_create_cube_map_from_pixels(VulkanContext* context, Texture* texture,
                             unsigned char* pixels[6], int width, int height,
                             VkFormat format);

bool vk_create_texture_image_view(VulkanContext* context, Texture* texture, VkFormat format, VkImageAspectFlags aspectFlags, VkImageViewType viewType);
bool vk_create_texture_sampler(VulkanContext* context, Texture* texture);
    
void vk_update_texture(VulkanContext* context, Texture* texture, VkFormat format, VkImageViewType viewType);
VkResult vk_update_texture_fast(VulkanContext* context, Texture* texture, VkFormat format, VkImageViewType viewType);

void vk_set_texture_pixel(Texture* texture, int x, int y, unsigned char r, unsigned char g, unsigned char b, unsigned char a, VkFormat format);
void vk_destroy_texture(VkDevice device, Texture* texture);




/*bool loadTextureFromFile(VulkanContext* context, Texture* texture,
                             const char* texturePath, VkFormat format, VkImageViewType viewType);

unsigned char* loadTextureFromFile(VulkanContext* context, int* width, int* height, 
                        const char* filepath, VkFormat format, 
                        VkImageViewType viewType);*/

bool createEmptyTexture(VulkanContext* context, Texture* texture,
                       int width, int height, VkFormat format,
                       VkImageViewType viewType);

void vk_create_staging_buffer_for_screenshot(VkDeviceSize size, VkBuffer* buffer, VkDeviceMemory* bufferMemory, void** mappedData);
void vk_destroy_staging_buffer_for_screenshot(VkBuffer buffer, VkDeviceMemory bufferMemory, void* mappedData);
void vk_take_screenshot(uint32_t imageIndex, const char* filename);



void updateTexture(VulkanContext* context, Texture* texture, VkFormat format, VkImageViewType viewType);

//////////////////////////////////////////////////////////////////


// #include "vulkan_utils.h"

VkResult fastTransitionImageLayout(VulkanContext* context, VkCommandBuffer commandBuffer, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageViewType viewType);
VkResult fastCopyBufferToImage(VulkanContext* context, VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, VkImageViewType viewType);

VkResult fastUpdateTexture(VulkanContext* context, Texture* texture, VkFormat format, VkImageViewType viewType);

#endif // VULKAN_IMAGE_H