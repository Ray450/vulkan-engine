#include "vulkan_device.h"
#include "vulkan_utils.h"
#include "vulkan_surface.h"
#include "../logger.h"

void vk_pick_physical_device(VkPhysicalDevice* physicalDevice, VkPhysicalDeviceLimits* deviceLimits, VkInstance instance, VkSurfaceKHR surface, const char* deviceExtensions[], size_t deviceExtensionCount, bool enableValidationLayers) {
    uint32_t deviceCount = 0;
    VK_CHECK(vkEnumeratePhysicalDevices(instance, &deviceCount, NULL));

    if (deviceCount == 0) {
        LOG_FATAL("No GPUs with Vulkan support found.");
    }

    VkPhysicalDevice* devices = (VkPhysicalDevice*)malloc(deviceCount * sizeof(VkPhysicalDevice));
    if (!devices) {
        LOG_FATAL("Memory allocation failed for physical devices.");
    }

    VK_CHECK(vkEnumeratePhysicalDevices(instance, &deviceCount, devices));

    VkPhysicalDeviceProperties deviceProperties;
    *physicalDevice = VK_NULL_HANDLE;

    for (size_t i = 0; i < deviceCount; ++i) {
        VkPhysicalDevice device = devices[i];

        // Inline isDeviceSuitable logic
        QueueFamilyIndices indices = vk_find_queue_families(surface, device);

        if (!indices.hasGraphicsFamily || !indices.hasPresentFamily || !indices.hasComputeFamily) {
            LOG_WARN("Device does not have required queue families.");
            continue;
        }

        bool extensionsSupported = vk_check_device_extension_support(device, deviceExtensions, deviceExtensionCount);
        if (!extensionsSupported) {
            LOG_WARN("Device does not support required extensions.");
            continue;
        }

        SwapChainSupportDetails swapChainSupport = vk_query_swap_chain_support(surface, device);
        bool swapChainAdequate = swapChainSupport.formatCount > 0 && swapChainSupport.presentModeCount > 0;
        if (!swapChainAdequate) {
            LOG_WARN("Swapchain support is inadequate.");
            continue;
        }

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);
        if (!supportedFeatures.samplerAnisotropy) {
            LOG_WARN("Device does not support sampler anisotropy.");
            continue;
        }

        // Device is suitable
        *physicalDevice = device;

        // Query properties and limits
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        *deviceLimits = deviceProperties.limits;

        if (enableValidationLayers) {
            char deviceName[256];
            snprintf(deviceName, sizeof(deviceName), "%s", deviceProperties.deviceName);

            LOG_INFO("Graphics | Present | Compute | Name");
            LOG_INFO("        %d |       %d |       %d | %s",
                indices.hasGraphicsFamily,
                indices.hasPresentFamily,
                indices.hasComputeFamily,
                deviceName);

            LOG_INFO("Selected device: '%s'.", deviceName);

            // Log key limits
            LOG_INFO("Device Limits:");
            LOG_INFO("  Max Image Dimension 2D: %u", deviceLimits->maxImageDimension2D);
            LOG_INFO("  Max Storage Buffer Range: %llu", deviceLimits->maxStorageBufferRange);
            LOG_INFO("  Max Uniform Buffer Range: %llu", deviceLimits->maxUniformBufferRange);
            LOG_INFO("  Max Push Constants Size: %u", deviceLimits->maxPushConstantsSize);
            LOG_INFO("  Max Sampler Anisotropy: %f", deviceLimits->maxSamplerAnisotropy);
        }

        break;
    }

    free(devices);

    if (*physicalDevice == VK_NULL_HANDLE) {
        LOG_FATAL("Failed to find a suitable GPU.");
    }
}




void vk_create_logical_device(VkDevice* device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkQueue* graphicsQueue, VkQueue* presentQueue, VkQueue* computeQueue, const char* deviceExtensions[], size_t deviceExtensionCount, const char* validationLayers[], size_t validationLayerCount, bool enableValidationLayers) {
    QueueFamilyIndices indices = vk_find_queue_families(surface, physicalDevice);

    if (!indices.hasGraphicsFamily || !indices.hasComputeFamily) {
        LOG_FATAL("Failed to find valid queue families for graphics or compute.");
    }
    
    uint32_t uniqueQueueFamilies[3]; // Added compute family
    size_t uniqueQueueFamilyCount = 0;

    if (indices.hasGraphicsFamily) {
        uniqueQueueFamilies[uniqueQueueFamilyCount++] = indices.graphicsFamily;
    }
    if (indices.hasPresentFamily && indices.presentFamily != indices.graphicsFamily) {
        uniqueQueueFamilies[uniqueQueueFamilyCount++] = indices.presentFamily;
    }
    if (indices.hasComputeFamily && indices.computeFamily != indices.graphicsFamily) {
        uniqueQueueFamilies[uniqueQueueFamilyCount++] = indices.computeFamily;
    }

    VkDeviceQueueCreateInfo queueCreateInfos[3];
    float queuePriority = 1.0f;
    for (size_t i = 0; i < uniqueQueueFamilyCount; i++) {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = uniqueQueueFamilies[i];
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos[i] = queueCreateInfo;
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = (uint32_t)uniqueQueueFamilyCount;
    createInfo.pQueueCreateInfos = queueCreateInfos;
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = (uint32_t)deviceExtensionCount;
    createInfo.ppEnabledExtensionNames = deviceExtensions;

    if (enableValidationLayers) {
        createInfo.enabledLayerCount = (uint32_t)validationLayerCount;
        createInfo.ppEnabledLayerNames = validationLayers;
    } else {
        createInfo.enabledLayerCount = 0;
    }

    VK_CHECK(vkCreateDevice(physicalDevice, &createInfo, NULL, device));

    vkGetDeviceQueue(*device, indices.graphicsFamily, 0, graphicsQueue);
    
    vkGetDeviceQueue(*device, indices.presentFamily, 0, presentQueue);
    
    vkGetDeviceQueue(*device, indices.computeFamily, 0, computeQueue);
    
}

