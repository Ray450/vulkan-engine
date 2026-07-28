
#include "vulkan_utils.h"

uint32_t vk_find_memory_type(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    LOG_FATAL("failed to find suitable memory type!");
}



void vk_create_image(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage* image, VkDeviceMemory* imageMemory, VkImageViewType viewType){    
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    
    //imageInfo.arrayLayers = 6;

    if (viewType == VK_IMAGE_VIEW_TYPE_CUBE) {
        imageInfo.arrayLayers = 6;
        imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    } else {
        imageInfo.arrayLayers = 1;
        imageInfo.flags = 0;
    }


    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK(vkCreateImage(device, &imageInfo, NULL, image));

    LOG_INFO("Created image: %ux%u (%llu bytes est.), limit %u; Total images: %u", width, height, (VkDeviceSize)width * height * 4, context.deviceLimits.maxImageDimension2D, ++context.totalImages);
    context.totalBytesAllocated += (VkDeviceSize)width * height * 4;

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, *image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = vk_find_memory_type(physicalDevice, memRequirements.memoryTypeBits, properties);

    VK_CHECK(vkAllocateMemory(device, &allocInfo, NULL, imageMemory));

    context.totalAllocations++;

    VK_CHECK(vkBindImageMemory(device, *image, *imageMemory, 0));
}

void vk_transition_image_layout(VulkanContext* context, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageViewType viewType) {
    VkCommandBuffer commandBuffer = vk_begin_single_time_commands(context);

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    
    //barrier.subresourceRange.layerCount = 1;

    if (viewType == VK_IMAGE_VIEW_TYPE_CUBE) {
        barrier.subresourceRange.layerCount = 6;
    } else {
        barrier.subresourceRange.layerCount = 1;
    }


    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

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
        LOG_ERROR("unsupported layout transition!");
        return;
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, NULL,
        0, NULL,
        1, &barrier
    );


    vk_end_single_time_commands(context, commandBuffer);
}

void vk_copy_buffer_to_image(VulkanContext* context, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    VkCommandBuffer commandBuffer = vk_begin_single_time_commands(context);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {
        width,
        height,
        1
    };

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    vk_end_single_time_commands(context, commandBuffer);
}

QueueFamilyIndices vk_find_queue_families(VkSurfaceKHR surface, VkPhysicalDevice device) {
    QueueFamilyIndices indices = {0};  // Initialize all members to zero

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);

    // Assume the maximum number of queue families is 32
    VkQueueFamilyProperties queueFamilies[32]; 
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);

    for (int i = 0; i < queueFamilyCount; i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
            indices.hasGraphicsFamily = 1;  // Set flag to true
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

        if (presentSupport) {
            indices.presentFamily = i;
            indices.hasPresentFamily = 1;  // Set flag to true
        }

        if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
            indices.computeFamily = i;
            indices.hasComputeFamily = 1;  // Set flag to true
        }

        // Stop searching once all are found
        if (indices.hasGraphicsFamily && indices.hasPresentFamily && indices.hasComputeFamily) {
            break;
        }
    }

    return indices;
}

bool vk_check_device_extension_support(VkPhysicalDevice device, 
                                  const char* deviceExtensions[], 
                                  size_t deviceExtensionCount) {
    uint32_t extensionCount;
    VK_CHECK(vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, NULL));
    // Dynamically allocate memory for available extensions
    VkExtensionProperties* availableExtensions = (VkExtensionProperties*)malloc(extensionCount * sizeof(VkExtensionProperties));
    if (availableExtensions == NULL) {
        LOG_FATAL("Memory allocation failed for available extensions.");
    }

    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, availableExtensions);

    // Dynamically allocate memory for required extensions
    char** requiredExtensions = (char**)malloc(deviceExtensionCount * sizeof(char*));
    if (requiredExtensions == NULL) {
        free(availableExtensions);
        LOG_FATAL("Memory allocation failed for required extensions.");
    }

    // Copy required extensions into the array
    for (size_t i = 0; i < deviceExtensionCount; ++i) {
        requiredExtensions[i] = (char*)malloc(strlen(deviceExtensions[i]) + 1);
        if (requiredExtensions[i] == NULL) {
            // Free already allocated memory before returning
            for (size_t j = 0; j < i; ++j) {
                free(requiredExtensions[j]);
            }
            free(requiredExtensions);
            free(availableExtensions);
            LOG_FATAL("Memory allocation failed for required extension: %s", deviceExtensions[i]);
        }
        strcpy(requiredExtensions[i], deviceExtensions[i]);
    }

    // Check if each required extension is available
    bool allExtensionsSupported = true;
    for (size_t i = 0; i < deviceExtensionCount; ++i) {
        bool found = false;
        for (uint32_t j = 0; j < extensionCount; ++j) {
            if (strcmp(availableExtensions[j].extensionName, requiredExtensions[i]) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            LOG_WARN("Required extension not found: %s", requiredExtensions[i]);
            allExtensionsSupported = false;
            break;
        }
    }

    // Clean up allocated memory
    for (size_t i = 0; i < deviceExtensionCount; ++i) {
        free(requiredExtensions[i]);
    }
    free(requiredExtensions);
    free(availableExtensions);

    return allExtensionsSupported;
}

SwapChainSupportDetails vk_query_swap_chain_support(VkSurfaceKHR surface, VkPhysicalDevice device) {
    SwapChainSupportDetails details = {0};  // Initialize to zero

    // Get surface capabilities
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities));

    // Get surface formats
    uint32_t formatCount;
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, NULL));
    
    if (formatCount > 0) {
        details.formatCount = formatCount;
        VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats));
    
    }

    // Get present modes
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, NULL);
    if (presentModeCount > 0) {
        details.presentModeCount = presentModeCount;
        VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes));
    }

    return details;
}

VkImageView vk_create_image_view(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageViewType viewType) {
        
    if (device == VK_NULL_HANDLE || image == VK_NULL_HANDLE) {
        LOG_FATAL("Invalid device or image handle in vk_create_image_view!");
    }
    
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = viewType;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;

    if(viewType == VK_IMAGE_VIEW_TYPE_2D) {
        viewInfo.subresourceRange.layerCount = 1;
    } else {
        viewInfo.subresourceRange.layerCount = 6;
    }   

    VkImageView imageView;
    VK_CHECK(vkCreateImageView(device, &viewInfo, NULL, &imageView));

    return imageView;
}


VkFormat vk_find_supported_format(VkPhysicalDevice physicalDevice, VkFormat* candidates, size_t candidateCount, VkImageTiling tiling, VkFormatFeatureFlags features) {
    
    if (physicalDevice == VK_NULL_HANDLE) {
        LOG_FATAL("vk_find_supported_format: Physical device is NULL!");
    }

    if (candidates == NULL || candidateCount == 0) {
        LOG_FATAL("vk_find_supported_format: Invalid candidates array or zero candidates!");
    }

    for (size_t i = 0; i < candidateCount; ++i) {
        VkFormat format = candidates[i];
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    LOG_FATAL("Failed to find a supported format!");
}

VkFormat vk_find_depth_format(VkPhysicalDevice physicalDevice) {
    VkFormat candidates[] = {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT};
    size_t candidateCount = sizeof(candidates) / sizeof(candidates[0]);

    return vk_find_supported_format(physicalDevice, candidates, candidateCount, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

bool vk_has_stencil_component(VkFormat format) {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

VkCommandBuffer vk_begin_single_time_commands(VulkanContext* context) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = context->commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    VK_CHECK(vkAllocateCommandBuffers(context->device, &allocInfo, &commandBuffer));
    
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));
    return commandBuffer;
}


void endSingleTimeCommands2(VulkanContext* context, VkCommandBuffer commandBuffer) {
    VK_CHECK(vkEndCommandBuffer(commandBuffer));

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VK_CHECK(vkQueueSubmit(context->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));
    
    VK_CHECK(vkQueueWaitIdle(context->graphicsQueue));

    vkFreeCommandBuffers(context->device, context->commandPool, 1, &commandBuffer);
}


VkFence singleTimeFence;


void vk_end_single_time_commands(VulkanContext* context, VkCommandBuffer commandBuffer) { // , uint32_t frameIndex) {
    VK_CHECK(vkEndCommandBuffer(commandBuffer));
    
    // Reset the fence before using it
    VK_CHECK(vkResetFences(context->device, 1, &context->singleTimeFence));
    
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    
    // Submit with your pre-allocated fence
    VK_CHECK(vkQueueSubmit(context->graphicsQueue, 1, &submitInfo, context->singleTimeFence));
    
    // Wait for the fence instead of queue idle
    VK_CHECK(vkWaitForFences(context->device, 1, &context->singleTimeFence, VK_TRUE, UINT64_MAX));
    
    vkFreeCommandBuffers(context->device, context->commandPool, 1, &commandBuffer);
}



VkShaderModule vk_create_shader_module(VkDevice device, const char* code, size_t codeSize) {
    
    if (device == VK_NULL_HANDLE) {
        LOG_FATAL("vk_create_shader_module: Device is NULL!");
    }

    if (code == NULL || codeSize == 0) {
        LOG_FATAL("vk_create_shader_module: Invalid shader code or code size!");
    }
    
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = codeSize;
    createInfo.pCode = (const uint32_t*)code; // Casting the raw data buffer to uint32_t*

    VkShaderModule shaderModule;
    VK_CHECK(vkCreateShaderModule(device, &createInfo, NULL, &shaderModule));

    return shaderModule;
}

void vk_destroy_debug_utils_messenger_ext(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != NULL) {
        func(instance, debugMessenger, pAllocator);
    }
}


