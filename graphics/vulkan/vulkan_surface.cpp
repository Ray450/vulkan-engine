#include "vulkan_surface.h"

const char* validationLayers[] = {
    "VK_LAYER_KHRONOS_validation"
};

const char* deviceExtensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    // "VK_KHR_shader_non_semantic_info",
    // "VK_EXT_debug_printf"
};

const size_t validationLayerCount = 1;
const size_t deviceExtensionCount = 1;


VulkanContext context = {
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


#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif


// #define TARGET_FPS 60  // Target frames per second
// #define TARGET_FRAME_TIME_MS (1000 / TARGET_FPS)  // Target time per frame in milliseconds


// GLFWWindowContext window;


/////////////////////////////////////////////////////////////////////////////////////////////


// #include "vulkan_device.h"



// Initializtion 
/*
    createWindow
    backend_createInstance
    backend_setupDebugMessenger
    backend_createSurface
    backend_pickPhysicalDevice
    backend_createLogicalDevice
    backend_createSwapChain
    backend_createImageViews
    backend_createRenderPass
    backend_createDepthResources
    backend_createFramebuffers
    backend_createSyncObjects
    backend_createDescriptorSetLayouts
    backend_createCommandPool
    backend_createCommandBuffers
*/

// extern int verbose_vk; // e.g., set this to 1 to enable verbose logs

int verbose_vk = 1;

const char* vkResultToString(VkResult result) {
    switch (result) {
        case VK_SUCCESS: return "VK_SUCCESS";
        case VK_NOT_READY: return "VK_NOT_READY";
        case VK_TIMEOUT: return "VK_TIMEOUT";
        case VK_EVENT_SET: return "VK_EVENT_SET";
        case VK_EVENT_RESET: return "VK_EVENT_RESET";
        case VK_INCOMPLETE: return "VK_INCOMPLETE";
        case VK_ERROR_OUT_OF_HOST_MEMORY: return "VK_ERROR_OUT_OF_HOST_MEMORY";
        case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
        case VK_ERROR_INITIALIZATION_FAILED: return "VK_ERROR_INITIALIZATION_FAILED";
        case VK_ERROR_DEVICE_LOST: return "VK_ERROR_DEVICE_LOST";
        case VK_ERROR_MEMORY_MAP_FAILED: return "VK_ERROR_MEMORY_MAP_FAILED";
        case VK_ERROR_LAYER_NOT_PRESENT: return "VK_ERROR_LAYER_NOT_PRESENT";
        case VK_ERROR_EXTENSION_NOT_PRESENT: return "VK_ERROR_EXTENSION_NOT_PRESENT";
        case VK_ERROR_FEATURE_NOT_PRESENT: return "VK_ERROR_FEATURE_NOT_PRESENT";
        case VK_ERROR_INCOMPATIBLE_DRIVER: return "VK_ERROR_INCOMPATIBLE_DRIVER";
        case VK_ERROR_TOO_MANY_OBJECTS: return "VK_ERROR_TOO_MANY_OBJECTS";
        case VK_ERROR_FORMAT_NOT_SUPPORTED: return "VK_ERROR_FORMAT_NOT_SUPPORTED";
        case VK_ERROR_FRAGMENTED_POOL: return "VK_ERROR_FRAGMENTED_POOL";
        case VK_ERROR_SURFACE_LOST_KHR: return "VK_ERROR_SURFACE_LOST_KHR";
        default: return "UNKNOWN_VK_RESULT";
    }
}












//------------------------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------------------------








//////////////////////////////////////////////////////////////////////////////////////////////



#define MAX_LAYERS 64

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    // std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    // Log the message with appropriate severity
    switch (messageSeverity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            logMessage(LOG_LEVEL_DEBUG, "Vulkan Debug (Verbose): %s", pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            logMessage(LOG_LEVEL_INFO, "Vulkan Info: %s", pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            logMessage(LOG_LEVEL_WARNING, "Vulkan Warning: %s", pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            logMessage(LOG_LEVEL_ERROR, "Vulkan Error: %s", pCallbackData->pMessage);
            break;
        default:
            logMessage(LOG_LEVEL_WARNING, "Vulkan Unknown Severity: %s", pCallbackData->pMessage);
            break;
    }

    return VK_FALSE;
}

void vk_create_instance(VkInstance* instance, bool enableValidationLayers, 
                             const char* validationLayers[],
                             size_t validationLayerCount) {


    uint32_t supportedApiVersion = 0;
    // Check what the loader/driver actually supports
    if (vkEnumerateInstanceVersion(&supportedApiVersion) == VK_SUCCESS) {
        uint32_t major = VK_API_VERSION_MAJOR(supportedApiVersion);
        uint32_t minor = VK_API_VERSION_MINOR(supportedApiVersion);
        
        if (major < 1 || (major == 1 && minor < 3)) {
            LOG_WARN("System supports Vulkan %d.%d, but 1.3 is recommended for this engine.", major, minor);
        } else {
            LOG_INFO("System supports Vulkan %d.%d (Meets recommendations).", major, minor);
        }
    }
    
    //================== VALIDATE LAYERS ==================
    if (enableValidationLayers) {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, NULL);
        
        VkLayerProperties availableLayers[MAX_LAYERS];
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);
        
        if (layerCount > MAX_LAYERS) {
            LOG_WARN("Too many Vulkan layers (%u). Clamping to %u.", layerCount, MAX_LAYERS);
            layerCount = MAX_LAYERS;
        }
        
        LOG_INFO("Available Vulkan layers: %u", layerCount);
        
        // Check if all requested validation layers are available
        for (size_t i = 0; i < validationLayerCount; ++i) {
            const char* layerName = validationLayers[i];
            bool layerFound = false;
            
            for (uint32_t j = 0; j < layerCount; ++j) {
                if (strcmp(layerName, availableLayers[j].layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }
            
            if (!layerFound) {
                LOG_FATAL("Validation layers requested, but not available!");
            }
        }
    }
    
    //================== GET REQUIRED EXTENSIONS ==================
    const size_t maxExtensionsCount = 10;
    const char* extensions[maxExtensionsCount];
    size_t extensionCount;
    {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        
        extensionCount = glfwExtensionCount;
        for (size_t i = 0; i < glfwExtensionCount && i < maxExtensionsCount; ++i) {
            extensions[i] = glfwExtensions[i];
        }
        
        if (enableValidationLayers && extensionCount < maxExtensionsCount) {
            extensions[extensionCount++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
        }
    }
    
    //================== SETUP APPLICATION INFO ==================
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;
    
    //================== SETUP INSTANCE CREATE INFO ==================
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensionCount);
    createInfo.ppEnabledExtensionNames = extensions;
    
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayerCount);
        createInfo.ppEnabledLayerNames = validationLayers;
                
        memset(&debugCreateInfo, 0, sizeof(debugCreateInfo));
        debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugCreateInfo.pfnUserCallback = debugCallback;
        debugCreateInfo.pUserData = NULL;  // optional
        debugCreateInfo.flags = 0;            // required to be zero


        
        // createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;

        createInfo.pNext = &debugCreateInfo;

    } else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = NULL;
    }
    
    //================== CREATE INSTANCE ==================
    VK_CHECK(vkCreateInstance(&createInfo, NULL, instance));
}

void vk_setup_debug_messenger(VkInstance instance, VkDebugUtilsMessengerEXT* debugMessenger) {
    if (!enableValidationLayers) return;


    //==================  ==================
    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    memset(&createInfo, 0, sizeof(createInfo));
    
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = NULL;  // optional
    createInfo.flags = 0;            // required to be zero

    //================== Call Vulkan function to create the messenger ==================
    PFN_vkCreateDebugUtilsMessengerEXT func =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != NULL) {
        if (func(instance, &createInfo, NULL, debugMessenger) != VK_SUCCESS) {
            LOG_FATAL("Failed to set up debug messenger. Exiting...");
        }
    } else {
        LOG_FATAL("vkCreateDebugUtilsMessengerEXT not found. Exiting...");
    }

}

void vk_create_surface(VkInstance instance, GLFWwindow* window, VkSurfaceKHR* surface) {
    VK_CHECK(glfwCreateWindowSurface(instance, window, NULL, surface));
}