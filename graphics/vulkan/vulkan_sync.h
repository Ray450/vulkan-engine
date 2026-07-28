#ifndef VULKAN_SYNC_H
#define VULKAN_SYNC_H

#include "vulkan_types.h"

void vk_create_sync_objects(VkDevice device, VkSemaphore* imageAvailableSemaphores, VkSemaphore* renderFinishedSemaphores, VkFence* inFlightFences, VkSemaphore* computeImageAvailableSemaphores, VkSemaphore* computeFinishedSemaphores, VkFence* computeInFlightFences, size_t maxFramesInFlight);
void vk_create_single_time_fence(VkDevice device, VkFence* singleTimeFence);


#endif // VULKAN_SYNC_H
