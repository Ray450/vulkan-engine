#include "vulkan_command.h"

void vk_create_command_pool(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkCommandPool* commandPool) {
    QueueFamilyIndices queueFamilyIndices = vk_find_queue_families(surface, physicalDevice);

    if (!queueFamilyIndices.hasGraphicsFamily) {
        LOG_FATAL("Graphics queue family not found!");
    }

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily; // Direct access to graphicsFamily

    if (vkCreateCommandPool(device, &poolInfo, NULL, commandPool) != VK_SUCCESS) {
        LOG_FATAL("failed to create graphics command pool!");
    }
}

void vk_create_command_buffers(VkDevice device, VkCommandPool commandPool, VkCommandBuffer* commandBuffers, uint32_t maxFramesInFlight) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = maxFramesInFlight;

    VK_CHECK(vkAllocateCommandBuffers(device, &allocInfo, commandBuffers));
}

void vk_create_compute_command_buffers(VkDevice device, VkCommandPool commandPool, VkCommandBuffer* computeCommandBuffers, uint32_t maxFramesInFlight) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = maxFramesInFlight;

    VK_CHECK(vkAllocateCommandBuffers(device, &allocInfo, computeCommandBuffers));
}

void vk_set_viewport(float x, float y, float width, float height, float minDepth, float maxDepth) {
    VkViewport viewport;
    viewport.x = x;
    viewport.y = y;
    viewport.width = width;
    viewport.height = height;
    viewport.minDepth = minDepth;
    viewport.maxDepth = maxDepth;

    vkCmdSetViewport(context.commandBuffers[context.currentFrame], 0, 1, &viewport);

    VkRect2D scissor;
    scissor.offset = {(int32_t)x, (int32_t)y};
    scissor.extent = {(uint32_t)width, (uint32_t)height};
    vkCmdSetScissor(context.commandBuffers[context.currentFrame], 0, 1, &scissor);
}

void vk_draw(GraphicsObject* object, uint32_t indexCount, uint32_t instanceCount, 
                  uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) {
    vkCmdDrawIndexed(context.commandBuffers[context.currentFrame], 
        indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}
