#include "vulkan_sync.h"
#include "vulkan_surface.h"  // For context global
#include "vulkan_swapchain.h"
#include "vulkan_renderpass.h"
#include "../logger.h"

void vk_create_sync_objects(VkDevice device, VkSemaphore* imageAvailableSemaphores, VkSemaphore* renderFinishedSemaphores, VkFence* inFlightFences, VkSemaphore* computeImageAvailableSemaphores, VkSemaphore* computeFinishedSemaphores, VkFence* computeInFlightFences, size_t maxFramesInFlight) {
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < maxFramesInFlight; i++) {
        // Graphics sync objects
        VK_CHECK(vkCreateSemaphore(device, &semaphoreInfo, NULL, &imageAvailableSemaphores[i]));
        VK_CHECK(vkCreateSemaphore(device, &semaphoreInfo, NULL, &renderFinishedSemaphores[i]));
        VK_CHECK(vkCreateFence(device, &fenceInfo, NULL, &inFlightFences[i]));

        // Compute sync objects
        VK_CHECK(vkCreateSemaphore(device, &semaphoreInfo, NULL, &computeImageAvailableSemaphores[i]));
        VK_CHECK(vkCreateSemaphore(device, &semaphoreInfo, NULL, &computeFinishedSemaphores[i]));
        VK_CHECK(vkCreateFence(device, &fenceInfo, NULL, &computeInFlightFences[i]));
    }
}

void vk_create_single_time_fence(VkDevice device, VkFence* singleTimeFence) {
    VkFenceCreateInfo singleTimeFenceInfo{};
    singleTimeFenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    // Don't set SIGNALED bit - we want it unsignaled
    VK_CHECK(vkCreateFence(device, &singleTimeFenceInfo, NULL, singleTimeFence));
}
