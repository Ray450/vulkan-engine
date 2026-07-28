#ifndef VULKAN_TYPES_H
#define VULKAN_TYPES_H
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stddef.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include "../math/matrix.h"
#include "../logger.h"

#define MAX_FRAMES_IN_FLIGHT 20
#define NUM_OBJECTS 100
#define MAX_SWAPCHAIN_IMAGES 16

#define MAX_FORMATS 32
#define MAX_PRESENT_MODES 32

extern const bool enableValidationLayers;

typedef struct {
    uint32_t graphicsFamily;
    uint32_t presentFamily;
    uint32_t computeFamily;
    int hasGraphicsFamily;
    int hasPresentFamily;
    int hasComputeFamily;
} QueueFamilyIndices;

typedef struct {
    VkDeviceMemory memory;
    void* mapped;
    VkBuffer       handle;
    VkDeviceSize   allocated_size;
    VkDeviceSize   requested_size;
    uint32_t       memory_type_index;
    VkBufferUsageFlags    usage;
    VkMemoryPropertyFlags properties;
    bool           is_locked;

} VulkanBuffer;

struct alignas(16)Vertex {
    struct Vec3 pos;
    struct Vec4 color;
    struct Vec2 texCoord; 
    struct Vec3 normal;   
};

typedef enum {
    VERTEX_FORMAT_POS_COLOR,
    VERTEX_FORMAT_POS_COLOR_TEX,
    // VERTEX_FORMAT_POS_COLOR_NORM,
    VERTEX_FORMAT_POS_COLOR_TEX_NORM
} VertexFormat;

struct UniformBufferObject {
    struct mat4 model;
    struct mat4 view;
    struct mat4 proj;
    Vec2 iResolution;
    float iTime;
    float padding;
    
    float x;
    float y;
    float z;
    float width;
    float height;
    float depth;
    float rotationAngle;
};

struct StorageBufferObject {
    float data[256];
};

typedef struct {
    struct Vec4 offset;
    struct Vec4 color;
    struct Vec4 shape;
    struct Vec4 padding;
} PushConstantData;

typedef struct {
    uint32_t numInputs;
    uint32_t padding[3];
} ComputePushConstantData;

typedef struct {
    unsigned char* pixels;
    int width, height;
} PixelBuffer;

typedef struct {
    VkImage image;
    VkDeviceMemory imageMemory;
    VkImageView imageView;
    VkSampler sampler;
    int width, height;
    unsigned char* pixelData;
} Texture;

typedef struct Pipeline {
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    int vertexFormat;
    VkCullModeFlags cullMode;
    VkPrimitiveTopology topology;
    const char* vertShaderPath;
    const char* fragShaderPath;
} Pipeline;


struct GraphicsObject {
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    VulkanBuffer vertexBuffer2;
    Vertex*      vertices2;
    VulkanBuffer indexBuffer2;
    uint16_t*    indices2;
    VulkanBuffer uniformBuffers2[MAX_FRAMES_IN_FLIGHT];
    VulkanBuffer instanceBuffer2;
    VkDescriptorPool descriptorPool;
    VkDescriptorSet descriptorSets[MAX_FRAMES_IN_FLIGHT];
    Vertex* vertices;
    size_t vertexCount;
    uint16_t* indices;
    size_t indexCount;
    struct UniformBufferObject ubo;
    int vertexFormat;
    size_t vertexSize;
    PushConstantData pushConstants;
    VkPrimitiveTopology topology;
    Texture texture;
};

typedef struct ComputeObject {
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
    VkDescriptorPool descriptorPool;
    VkDescriptorSet descriptorSets[MAX_FRAMES_IN_FLIGHT];
    VulkanBuffer inputBuffers[MAX_FRAMES_IN_FLIGHT];
    VulkanBuffer outputBuffers[MAX_FRAMES_IN_FLIGHT];
    uint32_t dispatchCountX;
    uint32_t dispatchCountY;
    uint32_t dispatchCountZ;
    size_t inputSize;
    size_t outputSize;
    bool isInitialized;
} ComputeObject;

typedef struct {
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR formats[MAX_FORMATS];
    VkPresentModeKHR presentModes[MAX_PRESENT_MODES];
    uint32_t formatCount;
    uint32_t presentModeCount;
} SwapChainSupportDetails;

typedef struct {
    GLFWwindow* handle;
    int width;
    int height;
    bool framebufferResized;
    clock_t lastTime;
    int frameCount;
    float fps;
} GLFWWindowContext;

typedef struct {
    const char* validationLayers[1];
    const char* deviceExtensions[1];
    bool enableValidationLayers;
    GLFWwindow* window;
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice;
    VkPhysicalDeviceLimits deviceLimits;
    VkDevice device;
    uint32_t totalBuffers;
    uint32_t totalImages;
    uint32_t totalPipelines;
    uint32_t totalBytesAllocated;
    uint32_t totalAllocations;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkQueue computeQueue;
    VkSwapchainKHR swapChain;
    VkImage swapChainImages[MAX_SWAPCHAIN_IMAGES];
    VkImageView swapChainImageViews[MAX_SWAPCHAIN_IMAGES];
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    VkFramebuffer swapChainFramebuffers[MAX_SWAPCHAIN_IMAGES];
    VkRenderPass renderPass;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorSetLayout computeDescriptorSetLayout;
    VkCommandPool commandPool;
    VkCommandPool computeCommandPool;
    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;
    VkCommandBuffer commandBuffers[MAX_FRAMES_IN_FLIGHT];
    VkSemaphore imageAvailableSemaphores[MAX_FRAMES_IN_FLIGHT];
    VkSemaphore renderFinishedSemaphores[MAX_FRAMES_IN_FLIGHT];
    VkFence inFlightFences[MAX_FRAMES_IN_FLIGHT];
    VkSemaphore computeImageAvailableSemaphores[MAX_FRAMES_IN_FLIGHT];
    VkSemaphore computeFinishedSemaphores[MAX_FRAMES_IN_FLIGHT];
    VkFence computeInFlightFences[MAX_FRAMES_IN_FLIGHT];
    uint32_t currentFrame;
    int objectIndex;
    bool framebufferResized;
    size_t swapChainImageCount;
    VkDescriptorPool computeDescriptorPool;
    VkBuffer storageBuffers[MAX_FRAMES_IN_FLIGHT];
    VkDeviceMemory storageBuffersMemory[MAX_FRAMES_IN_FLIGHT];
    void* storageBuffersMapped[MAX_FRAMES_IN_FLIGHT];
    VkDescriptorSet computeDescriptorSets[MAX_FRAMES_IN_FLIGHT];
    VkCommandBuffer computeCommandBuffers[MAX_FRAMES_IN_FLIGHT];
    struct mat4 view;
    struct mat4 proj;
    VulkanBuffer stagingBuffer;
    VkFence singleTimeFence;
} VulkanContext;

extern VulkanContext context;
const char* vkResultToString(VkResult result);

#ifdef NDEBUG
    // Release mode: minimal logging
    #define VK_CHECK(x)                                                        \
        do {                                                                   \
            VkResult err = (x);                                                \
            if (err != VK_SUCCESS) {                                           \
                LOG_FATAL("VK_CHECK failed: %s (%d)", vkResultToString(err), err); \
            }                                                                  \
        } while (0)
#else
    // Debug mode: verbose logging
    #define VK_CHECK(x)                                                        \
        do {                                                                   \
            VkResult err = (x);                                                \
            if (err != VK_SUCCESS) {                                           \
                LOG_FATAL("VK_CHECK failed: %s (%d)", vkResultToString(err), err); \
            }                                                                 \
        } while (0)




#define GLFW_CHECK(cond, msg)                                                       \
    do {                                                                            \
        if (!(cond)) {                                                              \
            logMessage(LOG_LEVEL_ERROR, "GLFW_CHECK failed: %s\n  Function: %s\n  File: %s:%d", \
                       msg, __func__, __FILE__, __LINE__);                          \
            return;                                                                 \
        }                                                                           \
    } while (0)


#endif

#endif // VULKAN_TYPES_H
