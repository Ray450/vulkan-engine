#include "vulkan_swapchain.h"

#include "vulkan_renderpass.h"

void vk_create_swap_chain(VkSwapchainKHR* swapChain, VkImage* swapChainImages, VkImageView* swapChainImageViews, VkFormat* swapChainImageFormat, VkExtent2D* swapChainExtent, VkFramebuffer* swapChainFramebuffers, size_t maxImageCount, size_t* actualImageCount, VkSurfaceKHR surface, VkPhysicalDevice physicalDevice, VkDevice device, VkRenderPass renderPass, GLFWwindow* window) {
    //================== Select swapchain surface format, present mode, and extent ==================

    SwapChainSupportDetails swapChainSupport = vk_query_swap_chain_support(surface, physicalDevice);

    if (swapChainSupport.formatCount == 0 || swapChainSupport.presentModeCount == 0) {
        LOG_FATAL("Swap chain support is insufficient! No formats or present modes available.");
    }

    // Choose surface format
    VkSurfaceFormatKHR surfaceFormat = swapChainSupport.formats[0];
    int foundFormat = 0;
    for (uint32_t i = 0; i < swapChainSupport.formatCount; ++i) {
        if (swapChainSupport.formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
            swapChainSupport.formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            surfaceFormat = swapChainSupport.formats[i];
            foundFormat = 1;
            break;
        }
    }

    if (!foundFormat) {
        LOG_WARN("Preferred format not found, using first available format.");
    }

    // Choose present mode
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    int foundPresent = 0;
    for (uint32_t i = 0; i < swapChainSupport.presentModeCount; ++i) {
        if (swapChainSupport.presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            presentMode = swapChainSupport.presentModes[i];
            foundPresent = 1;
            break;
        }
    }

    if (!foundPresent) {
        LOG_WARN("Mailbox present mode not available, using FIFO.");
    }

    // Choose extent
    VkExtent2D extent;
    if (swapChainSupport.capabilities.currentExtent.width != UINT32_MAX) {
        extent = swapChainSupport.capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        extent.width = (uint32_t)width;
        extent.height = (uint32_t)height;

        extent.width = extent.width < swapChainSupport.capabilities.minImageExtent.width ? swapChainSupport.capabilities.minImageExtent.width :
                        extent.width > swapChainSupport.capabilities.maxImageExtent.width ? swapChainSupport.capabilities.maxImageExtent.width :
                        extent.width;

        extent.height = extent.height < swapChainSupport.capabilities.minImageExtent.height ? swapChainSupport.capabilities.minImageExtent.height :
                         extent.height > swapChainSupport.capabilities.maxImageExtent.height ? swapChainSupport.capabilities.maxImageExtent.height :
                         extent.height;
    }

    //================== Create swapchain ==================

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    QueueFamilyIndices indices = vk_find_queue_families(surface, physicalDevice);
    if (!indices.hasGraphicsFamily || !indices.hasPresentFamily) {
        LOG_FATAL("Queue families not found!");
    }

    uint32_t queueFamilyIndices[] = {indices.graphicsFamily, indices.presentFamily};

    VkSwapchainCreateInfoKHR createInfo;
    memset(&createInfo, 0, sizeof(createInfo));
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = NULL;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    VK_CHECK(vkCreateSwapchainKHR(device, &createInfo, NULL, swapChain));

    uint32_t imageCountActual = 0;
    VK_CHECK(vkGetSwapchainImagesKHR(device, *swapChain, &imageCountActual, NULL));
    if (imageCountActual > maxImageCount) {
        LOG_WARN("Swap chain returned more images than expected. Limiting to %zu.", maxImageCount);
        imageCountActual = maxImageCount;
    }

    VK_CHECK(vkGetSwapchainImagesKHR(device, *swapChain, &imageCountActual, swapChainImages));

    *actualImageCount = imageCountActual;
    *swapChainImageFormat = surfaceFormat.format;
    *swapChainExtent = extent;
}

void vk_create_image_views(VkDevice device, VkImage* swapChainImages, VkImageView* swapChainImageViews, VkFormat swapChainImageFormat, size_t imageCount) {
    if (device == VK_NULL_HANDLE || swapChainImages == NULL || swapChainImageViews == NULL) {
        LOG_FATAL("Invalid parameters passed to vk_createImageViews!");
    }

    if (imageCount == 0) {
        LOG_FATAL("No images available to create image views!");
    }

    if (imageCount > MAX_SWAPCHAIN_IMAGES) {
        LOG_WARN("Image count (%zu) exceeds MAX_SWAPCHAIN_IMAGES (%d). Limiting to %d.", 
                   imageCount, MAX_SWAPCHAIN_IMAGES, MAX_SWAPCHAIN_IMAGES);
        imageCount = MAX_SWAPCHAIN_IMAGES; // Prevent out-of-bounds access
    }

    for (uint32_t i = 0; i < imageCount; i++) {
        //(*swapChainImageViews)[i] = 
        swapChainImageViews[i] = vk_create_image_view(device, swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D);
    }
}

// Windsurf generated cleanupSwapChain function
void vk_cleanup_swap_chain() {
    vkDestroyImageView(context.device, context.depthImageView, NULL);
    vkDestroyImage(context.device, context.depthImage, NULL);
    vkFreeMemory(context.device, context.depthImageMemory, NULL);

    for (size_t i = 0; i < context.swapChainImageCount; i++) {
        vkDestroyFramebuffer(context.device, context.swapChainFramebuffers[i], NULL);
    }

    // vkDestroyRenderPass(context.device, context.renderPass, NULL);

    for (size_t i = 0; i < context.swapChainImageCount; i++) {
        vkDestroyImageView(context.device, context.swapChainImageViews[i], NULL);
    }

    vkDestroySwapchainKHR(context.device, context.swapChain, NULL);
}/**/

// original cleanupSwapChain function
/*void cleanupSwapChain() {
    vkDestroyImageView(context.device, context.depthImageView, NULL);
    vkDestroyImage(context.device, context.depthImage, NULL);
    vkFreeMemory(context.device, context.depthImageMemory, NULL);

    for (auto framebuffer : context.swapChainFramebuffers) {
        vkDestroyFramebuffer(context.device, framebuffer, NULL);
    }

    for (auto imageView : context.swapChainImageViews) {
        vkDestroyImageView(context.device, imageView, NULL);
    }

    vkDestroySwapchainKHR(context.device, context.swapChain, NULL);
}/**/

void vk_recreate_swap_chain() {
    int width = 0, height = 0;
    size_t imageCount = 0;
    glfwGetFramebufferSize(context.window, &width, &height);

    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(context.window, &width, &height);
        glfwWaitEvents();
    }

    VK_CHECK(vkDeviceWaitIdle(context.device));

    vk_cleanup_swap_chain();

    vk_create_swap_chain(&context.swapChain, context.swapChainImages, context.swapChainImageViews, &context.swapChainImageFormat, &context.swapChainExtent, context.swapChainFramebuffers, MAX_SWAPCHAIN_IMAGES, &imageCount, context.surface, context.physicalDevice, context.device, context.renderPass, context.window);

    vk_create_image_views(context.device, context.swapChainImages, context.swapChainImageViews, context.swapChainImageFormat, imageCount);

    vk_create_render_pass(context.device, context.physicalDevice, context.swapChainImageFormat, &context.renderPass);

    vk_create_depth_resources(context.device, context.physicalDevice, context.swapChainExtent, &context.depthImage, &context.depthImageMemory, &context.depthImageView);

    vk_create_framebuffers(context.device, context.renderPass, context.swapChainImageViews, context.depthImageView, context.swapChainFramebuffers, context.swapChainExtent, imageCount);

    context.swapChainImageCount = imageCount;
}

uint32_t vk_acquire_next_image() {
    VK_CHECK(vkWaitForFences(context.device, 1, &context.inFlightFences[context.currentFrame], VK_TRUE, UINT64_MAX));
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(context.device, context.swapChain, UINT64_MAX, 
                                           context.imageAvailableSemaphores[context.currentFrame], 
                                           VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        vk_recreate_swap_chain();
        return UINT32_MAX; // Return invalid index to signal a retry
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        LOG_FATAL("failed to acquire swap chain image!");
    }
    
    VK_CHECK(vkResetFences(context.device, 1, &context.inFlightFences[context.currentFrame]));
    VK_CHECK(vkResetCommandBuffer(context.commandBuffers[context.currentFrame], 0));
    
    return imageIndex;
}

int vk_get_width() {
    return context.swapChainExtent.width;
}

int vk_get_height() {
    return context.swapChainExtent.height;
}

void vk_present_image(uint32_t imageIndex) {
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {context.imageAvailableSemaphores[context.currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &context.commandBuffers[context.currentFrame];

    VkSemaphore signalSemaphores[] = {context.renderFinishedSemaphores[context.currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    VK_CHECK(vkQueueSubmit(context.graphicsQueue, 1, &submitInfo, context.inFlightFences[context.currentFrame]));

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {context.swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    VkResult result = vkQueuePresentKHR(context.presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || context.framebufferResized) {
        context.framebufferResized = false;
        vk_recreate_swap_chain();
    } else if (result != VK_SUCCESS) {
        LOG_FATAL("failed to present swap chain image!");
    }

    context.currentFrame = (context.currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}