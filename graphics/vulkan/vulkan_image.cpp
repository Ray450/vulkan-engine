#include "vulkan_image.h"
#include "file_utils.h"

// ============================================================================

// LOW-LEVEL: Create cube map texture from 6 pixel buffers

// ============================================================================




bool vk_create_texture_from_pixels(VulkanContext* context, Texture* texture, 
                                   unsigned char* pixels, int width, int height, 
                                   VkFormat format, VkImageViewType viewType) {
    if (!pixels || width <= 0 || height <= 0) {
        logMessage(LOG_LEVEL_ERROR, "Invalid pixel data or dimensions");
        return false;
    }

    VkDeviceSize imageSize = width * height * 4; // RGBA

    // Store pixel data in texture
    texture->pixelData = (unsigned char*)malloc(imageSize);
    if (!texture->pixelData) {
        logMessage(LOG_LEVEL_ERROR, "Failed to allocate pixel data");
        return false;
    }
    memcpy(texture->pixelData, pixels, imageSize);

    // 1. Declare the new struct instead of separate handles
    VulkanBuffer stagingBuffer;
    
    // 2. Call the new vk_create_buffer function
    vk_create_buffer(context, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                  &stagingBuffer);

    // 3. Update vkMapMemory to use stagingBuffer.memory
    void* data;
    vkMapMemory(context->device, stagingBuffer.memory, 0, imageSize, 0, &data);
    memcpy(data, pixels, imageSize);
    vkUnmapMemory(context->device, stagingBuffer.memory);

    // Create GPU image
    vk_create_image(context->device, context->physicalDevice, width, height, format, VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &texture->image, &texture->imageMemory,
                viewType);

    // Transition layout and copy
    vk_transition_image_layout(context, texture->image, format, VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, viewType);
                          
    // 4. Update vk_copy_buffer_to_image to use stagingBuffer.handle
    vk_copy_buffer_to_image(context, stagingBuffer.handle, texture->image, (uint32_t)width, (uint32_t)height);
    
    vk_transition_image_layout(context, texture->image, format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, viewType);

    // 5. Cleanup using the struct's handle and memory
    vkDestroyBuffer(context->device, stagingBuffer.handle, NULL);
    vkFreeMemory(context->device, stagingBuffer.memory, NULL);

    texture->width = width;
    texture->height = height;

    return true;
}


bool vk_create_cube_map_from_pixels(VulkanContext* context, Texture* texture,
                             unsigned char* pixels[6], int width, int height,
                             VkFormat format) {

    logMessage(LOG_LEVEL_DEBUG, "vk_create_cube_map_from_pixels: %dx%d", width, height);

    VkDeviceSize layerSize = width * height * 4;
    VkDeviceSize imageSize = layerSize * 6;

    // Allocate combined pixel data
    texture->pixelData = (unsigned char*)malloc(imageSize);
    if (!texture->pixelData) {
        logMessage(LOG_LEVEL_ERROR, "Failed to allocate cube map pixel data");
        return false;
    }

    // Copy all 6 faces
    for (int i = 0; i < 6; i++) {
        memcpy((char*)texture->pixelData + i * layerSize, pixels[i], layerSize);
    }

    // 1. Declare the new struct instead of separate handles
    VulkanBuffer stagingBuffer;
    
    // 2. Call the new vk_create_buffer function
    vk_create_buffer(context, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                  &stagingBuffer);

    // 3. Update vkMapMemory to use stagingBuffer.memory
    void* data;
    vkMapMemory(context->device, stagingBuffer.memory, 0, imageSize, 0, &data);
    memcpy(data, texture->pixelData, imageSize);
    vkUnmapMemory(context->device, stagingBuffer.memory);

    // Create cube map image
    logMessage(LOG_LEVEL_DEBUG, "About to call vk_create_image...");

    vk_create_image(context->device, context->physicalDevice, width, height, format, VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &texture->image, &texture->imageMemory,
                VK_IMAGE_VIEW_TYPE_CUBE);

    if (texture->image == VK_NULL_HANDLE) {
        logMessage(LOG_LEVEL_ERROR, "Failed to create cube map image");
        return false;
    }

    logMessage(LOG_LEVEL_DEBUG, "Cube map image created successfully");

    // Transition and copy
    vk_transition_image_layout(context, texture->image, format, VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_VIEW_TYPE_CUBE);

    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 6;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {(uint32_t)width, (uint32_t)height, 1};

    VkCommandBuffer commandBuffer = vk_begin_single_time_commands(context);
    
    // 4. Update vkCmdCopyBufferToImage to use stagingBuffer.handle
    vkCmdCopyBufferToImage(commandBuffer, stagingBuffer.handle, texture->image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
                           
    vk_end_single_time_commands(context, commandBuffer);

    vk_transition_image_layout(context, texture->image, format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_VIEW_TYPE_CUBE);

    // 5. Cleanup using the struct's handle and memory
    vkDestroyBuffer(context->device, stagingBuffer.handle, NULL);
    vkFreeMemory(context->device, stagingBuffer.memory, NULL);
    
    // Note: Since vk_create_buffer increments context->totalBytesAllocated, 
    // you may eventually want a destroyBuffer2(context, &stagingBuffer) function 
    // to cleanly decrement your tracking variables instead of calling vkDestroy/Free directly.

    texture->width = width;
    texture->height = height;

    return true;
}

bool createEmptyTexture(VulkanContext* context, Texture* texture,

                       int width, int height, VkFormat format,

                       VkImageViewType viewType) {

    VkDeviceSize imageSize = width * height * 4;



    // Allocate blank pixel data (all zeros)

    unsigned char* blankPixels = (unsigned char*)calloc(imageSize, 1);

    if (!blankPixels) {

        logMessage(LOG_LEVEL_ERROR, "Failed to allocate blank pixel data");

        return false;

    }



    // Create texture from blank pixels

    bool result = vk_create_texture_from_pixels(context, texture, blankPixels, width, height, format, viewType);
    vk_create_texture_image_view(context, texture, format, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D);
    vk_create_texture_sampler(context, texture);


    free(blankPixels);



    return result;

}

bool vk_create_texture_image_view(VulkanContext* context, Texture* texture, VkFormat format, VkImageAspectFlags aspectFlags, VkImageViewType viewType) {

    texture->imageView = vk_create_image_view(context->device, texture->image, format, aspectFlags, viewType);

    if (texture->imageView == VK_NULL_HANDLE) {

        logMessage(LOG_LEVEL_ERROR, "Failed to create texture image view");

        return false;

    }



    return true;



}



bool vk_create_texture_sampler(VulkanContext* context, Texture* texture) {

    VkPhysicalDeviceProperties properties;

    vkGetPhysicalDeviceProperties(context->physicalDevice, &properties);



    VkSamplerCreateInfo samplerInfo;

    memset(&samplerInfo, 0, sizeof(samplerInfo));

    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

    samplerInfo.magFilter = VK_FILTER_LINEAR;

    samplerInfo.minFilter = VK_FILTER_LINEAR;

    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    samplerInfo.anisotropyEnable = VK_TRUE;

    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

    samplerInfo.unnormalizedCoordinates = VK_FALSE;

    samplerInfo.compareEnable = VK_FALSE;

    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;



    if (vkCreateSampler(context->device, &samplerInfo, NULL, &texture->sampler) != VK_SUCCESS) {

        logMessage(LOG_LEVEL_ERROR, "failed to create texture sampler!");

        return false;

    }



    return true;

}



void vk_set_texture_pixel(Texture* texture, int x, int y, unsigned char r, unsigned char g, unsigned char b, unsigned char a, VkFormat format) {

    

    int texWidth = texture->width, texHeight = texture->height, texChannels;



    // Ensure that (x, y) is within bounds

    /*if (x < 0 || x >= texture->width || y < 0 || y >= texture->height) {

        logMessage(LOG_LEVEL_ERROR, "Invalid pixel coordinates!");

        exit(EXIT_FAILURE);

    }*/



    if (!texture->pixelData) {

        LOG_FATAL("texture has no pixelData (already uploaded?)");

    }



    if (x < 0 || x >= texture->width || y < 0 || y >= texture->height) {

        LOG_FATAL("Invalid pixel coordinates: (%d, %d) out of [%d x %d]",

                   x, y, texture->width, texture->height);

    }



    // Update the pixel data

    int index = (y * texture->width + x) * 4; // 4 because RGBA is 4 channels

    texture->pixelData[index + 0] = r;  // Red

    texture->pixelData[index + 1] = g;  // Green

    texture->pixelData[index + 2] = b;  // Blue

    texture->pixelData[index + 3] = a;  // Alpha

}



void vk_update_texture(VulkanContext* context, Texture* texture, VkFormat format, VkImageViewType viewType) {
    
    int texWidth = texture->width, texHeight = texture->height, texChannels;

    VkDeviceSize imageSize = texWidth * texHeight * 4;

    // use a for loop that draws a rectangle inside the texture to test the pixel function

    // 1. Declare the new struct instead of separate handles
    VulkanBuffer stagingBuffer;
    
    // 2. Call the new vk_create_buffer function
    vk_create_buffer(context, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
                  &stagingBuffer);

    //exit(0);

    // 3. Update vkMapMemory and vkUnmapMemory to use stagingBuffer.memory
    void* data;
    vkMapMemory(context->device, stagingBuffer.memory, 0, imageSize, 0, &data);
        memcpy(data, texture->pixelData, static_cast<size_t>(imageSize));
    vkUnmapMemory(context->device, stagingBuffer.memory);

    // stbi_image_free(pixels);

    // vk_create_image(texWidth, texHeight, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

    vk_transition_image_layout(context, texture->image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, viewType);
        
    // 4. Update vk_copy_buffer_to_image to use stagingBuffer.handle
    vk_copy_buffer_to_image(context, stagingBuffer.handle, texture->image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
        
    vk_transition_image_layout(context, texture->image, format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, viewType);

    // 5. Cleanup using the struct's handle and memory
    vkDestroyBuffer(context->device, stagingBuffer.handle, NULL);
    vkFreeMemory(context->device, stagingBuffer.memory, NULL);
}



void updateTexture(VulkanContext* context, Texture* texture, VkFormat format, VkImageViewType viewType) {

    if (!texture || !texture->pixelData || !context->stagingBuffer.mapped) {

        logMessage(LOG_LEVEL_ERROR, "Invalid texture or staging buffer");

        return;

    }



    VkDeviceSize imageSize = texture->width * texture->height * 4;

    if (imageSize > context->stagingBuffer.allocated_size) {

        logMessage(LOG_LEVEL_ERROR, "Texture size (%zu bytes) exceeds staging buffer capacity (%zu bytes)", imageSize, context->stagingBuffer.allocated_size);

        return;

    }



    memcpy(context->stagingBuffer.mapped, texture->pixelData, imageSize);



    vk_transition_image_layout(context, texture->image, format,

                          VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,

                          viewType);



    vk_copy_buffer_to_image(context, context->stagingBuffer.handle, texture->image,

                      (uint32_t)texture->width, (uint32_t)texture->height);



    vk_transition_image_layout(context, texture->image, format,

                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,

                          viewType);



    logMessage(LOG_LEVEL_DEBUG, "Updated texture: %dx%d", texture->width, texture->height);

}



//////////////////////////////////////////////////////////////////
// Fast functions for texture updates
//////////////////////////////////////////////////////////////////




// #include "vulkan_utils.h"



VkResult vk_transition_image_layout_fast(VulkanContext* context, VkCommandBuffer commandBuffer, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageViewType viewType) {

    if (!context || !commandBuffer || image == VK_NULL_HANDLE) {

        logMessage(LOG_LEVEL_ERROR, "vk_transition_image_layout_fast: Invalid context, command buffer, or image!");

        return VK_ERROR_INITIALIZATION_FAILED;

    }



    VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    if (format >= VK_FORMAT_D16_UNORM && format <= VK_FORMAT_D32_SFLOAT_S8_UINT) {

        aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if (format >= VK_FORMAT_D16_UNORM_S8_UINT) {

            aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;

        }

    }



    VkImageSubresourceRange subresourceRange = {};

    subresourceRange.aspectMask = aspectMask;

    subresourceRange.baseMipLevel = 0;

    subresourceRange.levelCount = 1;

    subresourceRange.baseArrayLayer = 0;

    subresourceRange.layerCount = (viewType == VK_IMAGE_VIEW_TYPE_CUBE) ? 6 : 1;



    VkImageMemoryBarrier barrier = {};

    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

    barrier.oldLayout = oldLayout;

    barrier.newLayout = newLayout;

    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    barrier.image = image;

    barrier.subresourceRange = subresourceRange;



    VkPipelineStageFlags sourceStage, destinationStage;



    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {

        barrier.srcAccessMask = 0;

        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {

        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

    } else {

        logMessage(LOG_LEVEL_ERROR, "vk_transition_image_layout_fast: Unsupported layout transition: %d to %d!", oldLayout, newLayout);

        return VK_ERROR_FEATURE_NOT_PRESENT;

    }



    vkCmdPipelineBarrier(

        commandBuffer,

        sourceStage, destinationStage,

        0,

        0, NULL,

        0, NULL,

        1, &barrier

    );



    return VK_SUCCESS;

}



VkResult vk_copy_buffer_to_image_fast(VulkanContext* context, VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, VkImageViewType viewType) {

    if (!context || !commandBuffer || buffer == VK_NULL_HANDLE || image == VK_NULL_HANDLE || width == 0 || height == 0) {

        logMessage(LOG_LEVEL_ERROR, "vk_copy_buffer_to_image_fast: Invalid context, command buffer, or parameters!");

        return VK_ERROR_INITIALIZATION_FAILED;

    }



    VkBufferImageCopy region = {};

    region.bufferOffset = 0;

    region.bufferRowLength = 0;

    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    region.imageSubresource.mipLevel = 0;

    region.imageSubresource.baseArrayLayer = 0;

    region.imageSubresource.layerCount = (viewType == VK_IMAGE_VIEW_TYPE_CUBE) ? 6 : 1;

    region.imageOffset.x = 0;

    region.imageOffset.y = 0;

    region.imageOffset.z = 0;

    region.imageExtent.width = width;

    region.imageExtent.height = height;

    region.imageExtent.depth = 1;



    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    return VK_SUCCESS;

}



VkResult vk_update_texture_fast(VulkanContext* context, Texture* texture, VkFormat format, VkImageViewType viewType) {

    if (!context || !texture || !texture->pixelData || !context->stagingBuffer.handle || !context->stagingBuffer.mapped) {

        logMessage(LOG_LEVEL_ERROR, "vk_update_texture_fast: Invalid context, texture, or staging buffer!");

        return VK_ERROR_INITIALIZATION_FAILED;

    }



    VkDeviceSize bytesPerPixel = 4; // Default for VK_FORMAT_R8G8B8A8_UNORM

    if (format == VK_FORMAT_R8G8B8A8_SRGB || format == VK_FORMAT_R8G8B8A8_UNORM) {

        bytesPerPixel = 4;

    } else {

        logMessage(LOG_LEVEL_ERROR, "vk_update_texture_fast: Unsupported format!");

        return VK_ERROR_FEATURE_NOT_PRESENT;

    }



    VkDeviceSize imageSize = texture->width * texture->height * bytesPerPixel * (viewType == VK_IMAGE_VIEW_TYPE_CUBE ? 6 : 1);

    if (imageSize > context->stagingBuffer.allocated_size) {

        logMessage(LOG_LEVEL_ERROR, "Texture size (%zu bytes) exceeds staging buffer capacity (%zu bytes)!", imageSize, context->stagingBuffer.allocated_size);

        return VK_ERROR_OUT_OF_HOST_MEMORY;

    }



    // Allocate and begin command buffer

    VkCommandBufferAllocateInfo allocInfo = {};

    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;

    allocInfo.pNext = NULL;

    allocInfo.commandPool = context->commandPool;

    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    allocInfo.commandBufferCount = 1;



    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;

    if (vkAllocateCommandBuffers(context->device, &allocInfo, &commandBuffer) != VK_SUCCESS) {

        logMessage(LOG_LEVEL_ERROR, "vk_update_texture_fast: Failed to allocate command buffer!");

        return VK_ERROR_INITIALIZATION_FAILED;

    }



    VkCommandBufferBeginInfo beginInfo = {};

    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;



    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {

        logMessage(LOG_LEVEL_ERROR, "vk_update_texture_fast: Failed to begin command buffer!");

        vkFreeCommandBuffers(context->device, context->commandPool, 1, &commandBuffer);

        return VK_ERROR_INITIALIZATION_FAILED;

    }



    // Copy pixel data to staging buffer

    memcpy(context->stagingBuffer.mapped, texture->pixelData, imageSize);



    // Transition to TRANSFER_DST_OPTIMAL

    VkResult result = vk_transition_image_layout_fast(context, commandBuffer, texture->image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, viewType);

    if (result != VK_SUCCESS) {

        logMessage(LOG_LEVEL_ERROR, "vk_update_texture_fast: Failed to transition image to TRANSFER_DST!");

        vkFreeCommandBuffers(context->device, context->commandPool, 1, &commandBuffer);

        return result;

    }



    // Copy buffer to image

    result = vk_copy_buffer_to_image_fast(context, commandBuffer, context->stagingBuffer.handle, texture->image, texture->width, texture->height, viewType);

    if (result != VK_SUCCESS) {

        logMessage(LOG_LEVEL_ERROR, "vk_update_texture_fast: Failed to copy buffer to image!");

        vkFreeCommandBuffers(context->device, context->commandPool, 1, &commandBuffer);

        return result;

    }



    // Transition to SHADER_READ_ONLY_OPTIMAL

    result = vk_transition_image_layout_fast(context, commandBuffer, texture->image, format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, viewType);

    if (result != VK_SUCCESS) {

        logMessage(LOG_LEVEL_ERROR, "vk_update_texture_fast: Failed to transition image to SHADER_READ_ONLY!");

        vkFreeCommandBuffers(context->device, context->commandPool, 1, &commandBuffer);

        return result;

    }



    // End and submit command buffer

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {

        logMessage(LOG_LEVEL_ERROR, "vk_update_texture_fast: Failed to end command buffer!");

        vkFreeCommandBuffers(context->device, context->commandPool, 1, &commandBuffer);

        return VK_ERROR_INITIALIZATION_FAILED;

    }



    if (vkResetFences(context->device, 1, &context->singleTimeFence) != VK_SUCCESS) {

        logMessage(LOG_LEVEL_ERROR, "vk_update_texture_fast: Failed to reset single-time fence!");

        vkFreeCommandBuffers(context->device, context->commandPool, 1, &commandBuffer);

        return VK_ERROR_INITIALIZATION_FAILED;

    }



    VkSubmitInfo submitInfo = {};

    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    submitInfo.commandBufferCount = 1;

    submitInfo.pCommandBuffers = &commandBuffer;



    if (vkQueueSubmit(context->graphicsQueue, 1, &submitInfo, context->singleTimeFence) != VK_SUCCESS) {

        logMessage(LOG_LEVEL_ERROR, "vk_update_texture_fast: Failed to submit command buffer!");

        vkFreeCommandBuffers(context->device, context->commandPool, 1, &commandBuffer);

        return VK_ERROR_INITIALIZATION_FAILED;

    }



    if (vkWaitForFences(context->device, 1, &context->singleTimeFence, VK_TRUE, 1000000000) != VK_SUCCESS) {

        logMessage(LOG_LEVEL_ERROR, "vk_update_texture_fast: Failed to wait for single-time fence!");

        vkFreeCommandBuffers(context->device, context->commandPool, 1, &commandBuffer);

        return VK_ERROR_INITIALIZATION_FAILED;

    }



    vkFreeCommandBuffers(context->device, context->commandPool, 1, &commandBuffer);

    // logMessage(LOG_LEVEL_DEBUG, "Fast updated texture: %dx%d, viewType: %d", texture->width, texture->height, viewType);

    return VK_SUCCESS;

}



void vk_destroy_texture(VkDevice device, Texture* texture) {

    if (texture->sampler != VK_NULL_HANDLE) {

        vkDestroySampler(device, texture->sampler, NULL);

    }

    if (texture->imageView != VK_NULL_HANDLE) {

        vkDestroyImageView(device, texture->imageView, NULL);

    }

    if (texture->image != VK_NULL_HANDLE) {

        vkDestroyImage(device, texture->image, NULL);

    }

    if (texture->imageMemory != VK_NULL_HANDLE) {

        vkFreeMemory(device, texture->imageMemory, NULL);

    }



    texture->sampler = VK_NULL_HANDLE;

    texture->imageView = VK_NULL_HANDLE;

    texture->image = VK_NULL_HANDLE;

    texture->imageMemory = VK_NULL_HANDLE;

    if (texture->pixelData != NULL) {

        free(texture->pixelData);

        texture->pixelData = NULL;

    }
}

void vk_create_staging_buffer_for_screenshot(VkDeviceSize size, VulkanBuffer* outBuffer, void** mappedData) {
    // 1. Call vk_create_buffer using the struct pointer
    vk_create_buffer(&context, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                  outBuffer);

    // 2. Map memory using the struct's memory field
    vkMapMemory(context.device, outBuffer->memory, 0, size, 0, mappedData);
}

void vk_destroy_staging_buffer_for_screenshot(VkBuffer buffer, VkDeviceMemory bufferMemory, void* mappedData) {

    if (mappedData) vkUnmapMemory(context.device, bufferMemory);

    vkDestroyBuffer(context.device, buffer, NULL);

    vkFreeMemory(context.device, bufferMemory, NULL);

}

void vk_destroy_staging_buffer_for_screenshot(VulkanBuffer* buffer, void* mappedData) {
    if (mappedData) {
        vkUnmapMemory(context.device, buffer->memory);
    }
    vkDestroyBuffer(context.device, buffer->handle, NULL);
    vkFreeMemory(context.device, buffer->memory, NULL);
}

void vk_take_screenshot(uint32_t imageIndex, const char* filename) {
    VkImage srcImage = context.swapChainImages[imageIndex];
    VkExtent2D extent = context.swapChainExtent;
    VkFormat format = context.swapChainImageFormat;  // Assume R8G8B8A8_UNORM

    // Calculate buffer size (RGBA, 4 bytes/pixel)
    VkDeviceSize bufferSize = extent.width * extent.height * 4;

    // 1. Use the new VulkanBuffer struct instead of separate handles
    VulkanBuffer stagingBuffer = {};
    void* mappedData = NULL;
    
    // 2. Call the updated helper function
    vk_create_staging_buffer_for_screenshot(bufferSize, &stagingBuffer, &mappedData);

    // Create a fence to wait for rendering completion
    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    VkFence fence = VK_NULL_HANDLE;
    if (vkCreateFence(context.device, &fenceInfo, NULL, &fence) != VK_SUCCESS) {
        logMessage(LOG_LEVEL_ERROR, "Failed to create fence for screenshot synchronization");
        
        // 3. Update the early-exit cleanup 
        vk_destroy_staging_buffer_for_screenshot(&stagingBuffer, mappedData);
        return;
    }

    // Wait for the main render command buffer to complete
    vkQueueSubmit(context.graphicsQueue, 0, NULL, fence); // Ensure render commands complete before screenshot
    vkWaitForFences(context.device, 1, &fence, VK_TRUE, UINT64_MAX);
    vkDestroyFence(context.device, fence, NULL);

    // Begin a one-time command buffer for screenshot
    VkCommandBuffer cmdBuffer = vk_begin_single_time_commands(&context);

    // Log initial layout
    logMessage(LOG_LEVEL_INFO, "Screenshot: Starting with swapchain image in PRESENT_SRC_KHR");

    // Transition srcImage: PRESENT_SRC_KHR -> TRANSFER_SRC_OPTIMAL
    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = srcImage;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

    vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

    // Copy image to buffer
    VkBufferImageCopy copyRegion = {};
    copyRegion.bufferOffset = 0;
    copyRegion.bufferRowLength = 0;
    copyRegion.bufferImageHeight = 0;
    copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.imageSubresource.mipLevel = 0;
    copyRegion.imageSubresource.baseArrayLayer = 0;
    copyRegion.imageSubresource.layerCount = 1;
    copyRegion.imageOffset = {0, 0, 0};
    copyRegion.imageExtent = {extent.width, extent.height, 1};

    // 4. Update the copy command to use stagingBuffer.handle
    vkCmdCopyImageToBuffer(cmdBuffer, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, stagingBuffer.handle, 1, &copyRegion);

    // Transition back: TRANSFER_SRC_OPTIMAL -> PRESENT_SRC_KHR
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // Ensure PRESENT_SRC_KHR for presentation compatibility
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.dstAccessMask = 0;

    vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

    // Log final layout
    logMessage(LOG_LEVEL_INFO, "Screenshot: Transitioned swapchain image to PRESENT_SRC_KHR");

    vk_end_single_time_commands(&context, cmdBuffer);

    // Determine if the format is BGRA so we can fix the colors during save
    bool is_bgra = (format == VK_FORMAT_B8G8R8A8_UNORM || format == VK_FORMAT_B8G8R8A8_SRGB);
    
    // Hand off to our I/O function to handle the flip and write
    save_image_png(filename, (unsigned char*)mappedData, extent.width, extent.height, is_bgra);
    
    // 5. Update the final cleanup
    vk_destroy_staging_buffer_for_screenshot(&stagingBuffer, mappedData);
}