#ifndef VULKAN_SURFACE_H
#define VULKAN_SURFACE_H

#include "vulkan_types.h"
#include <cassert>

extern const char* validationLayers[];
extern const char* deviceExtensions[];
extern const size_t validationLayerCount;
extern const size_t deviceExtensionCount;

/*const char* validationLayers[] = {
    "VK_LAYER_KHRONOS_validation"
};

const char* deviceExtensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    // "VK_KHR_shader_non_semantic_info",
    // "VK_EXT_debug_printf"
};*/


extern VulkanContext context;

/*VulkanContext context = {
    .validationLayers = { "VK_LAYER_KHRONOS_validation" },
    .deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME },
#ifdef NDEBUG
    .enableValidationLayers = false,
#else
    .enableValidationLayers = true,
#endif
    .physicalDevice = VK_NULL_HANDLE,
    .currentFrame = 0,
    .framebufferResized = false

};/**/


/*#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif
*/

// #define TARGET_FPS 60  // Target frames per second
// #define TARGET_FRAME_TIME_MS (1000 / TARGET_FPS)  // Target time per frame in milliseconds


// GLFWWindowContext window;


/////////////////////////////////////////////////////////////////////////////////////////////


// #include "vulkan_device.h"



// Initializtion 
/*
    createWindow
    kv_createInstance
    kv_setupDebugMessenger
    kv_createSurface
    kv_pickPhysicalDevice
    kv_createLogicalDevice
    kv_createSwapChain
    kv_createImageViews
    kv_createRenderPass
    kv_createDepthResources
    kv_createFramebuffers
    kv_createSyncObjects
    kv_createDescriptorSetLayouts
    kv_createCommandPool
    kv_createCommandBuffers
*/

extern int verbose_vk; // e.g., set this to 1 to enable verbose logs

// int verbose_vk = 1;







//------------------------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------------------------








//////////////////////////////////////////////////////////////////////////////////////////////



#define MAX_LAYERS 64

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

void vk_create_instance(VkInstance* instance, bool enableValidationLayers, 
                             const char* validationLayers[],
                             size_t validationLayerCount);

void vk_setup_debug_messenger(VkInstance instance, VkDebugUtilsMessengerEXT* debugMessenger);

void vk_create_surface(VkInstance instance, GLFWwindow* window, VkSurfaceKHR* surface);
#endif

