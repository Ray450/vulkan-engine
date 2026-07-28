#include "vulkan_buffer.h"
#include "sys.h"



void vk_create_buffer(VulkanContext* context, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VulkanBuffer* outBuffer) {
    
    outBuffer->requested_size = size;
    outBuffer->usage          = usage;
    outBuffer->properties     = properties;

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size        = size;
    bufferInfo.usage       = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK(vkCreateBuffer(context->device, &bufferInfo, NULL, &outBuffer->handle));

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(context->device, outBuffer->handle, &memRequirements);

    outBuffer->allocated_size    = memRequirements.size;
    outBuffer->memory_type_index = vk_find_memory_type(context->physicalDevice, memRequirements.memoryTypeBits, properties);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize  = memRequirements.size;
    allocInfo.memoryTypeIndex = outBuffer->memory_type_index;

    VK_CHECK(vkAllocateMemory(context->device, &allocInfo, NULL, &outBuffer->memory));
    VK_CHECK(vkBindBufferMemory(context->device, outBuffer->handle, outBuffer->memory, 0));

    // update context tracking
    context->totalBytesAllocated += memRequirements.size;
    context->totalAllocations++;
}

void vk_copy_buffer(VulkanContext* context, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {

    if (dstBuffer == VK_NULL_HANDLE) {
        LOG_FATAL("vk_copy_buffer: dstBuffer is VK_NULL_HANDLE. srcBuffer: %p, size: %llu", (void*)srcBuffer, (unsigned long long)size);
    }
    if (srcBuffer == VK_NULL_HANDLE) {
        LOG_FATAL("vk_copy_buffer: srcBuffer is VK_NULL_HANDLE. dstBuffer: %p, size: %llu", (void*)dstBuffer, (unsigned long long)size);
    }


    VkCommandBuffer commandBuffer = vk_begin_single_time_commands(context);



    VkBufferCopy copyRegion{};

    copyRegion.size = size;

    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);



    vk_end_single_time_commands(context, commandBuffer);

}



void vk_init_staging_buffer(VulkanContext* context, VkDeviceSize maxSize) {
    context->stagingBuffer = {};

    vk_create_buffer(context, maxSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 &context->stagingBuffer);

    VK_CHECK(vkMapMemory(context->device, context->stagingBuffer.memory, 0, maxSize, 0, &context->stagingBuffer.mapped));
}

void vk_create_vertex_buffer(VulkanContext* context, Vertex* vertices, size_t vertexCount, VulkanBuffer* vertexBuffer, Vertex** mappedVertices, size_t vertexSize, VkMemoryPropertyFlags properties) {
    VkDeviceSize bufferSize = vertexCount * vertexSize;

    if (vertices == NULL) {
        LOG_FATAL("vertices == NULL - fatal error in vk_create_vertex_buffer");
    }

    bool isDeviceLocal = properties & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    if (isDeviceLocal) {
        VulkanBuffer stagingBuffer = {};
        vk_create_buffer(context, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &stagingBuffer);

        void* data;
        VK_CHECK(vkMapMemory(context->device, stagingBuffer.memory, 0, bufferSize, 0, &data));
        memcpy(data, vertices, bufferSize);
        vkUnmapMemory(context->device, stagingBuffer.memory);

        vk_create_buffer(context, bufferSize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            properties, vertexBuffer);

        vk_copy_buffer(context, stagingBuffer.handle, vertexBuffer->handle, bufferSize);

        vkDestroyBuffer(context->device, stagingBuffer.handle, NULL);
        vkFreeMemory(context->device, stagingBuffer.memory, NULL);

        if (mappedVertices) *mappedVertices = NULL;
    } else {
        vk_create_buffer(context, bufferSize,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            properties, vertexBuffer);

        void* data;
        VK_CHECK(vkMapMemory(context->device, vertexBuffer->memory, 0, bufferSize, 0, &data));
        memcpy(data, vertices, bufferSize);
        vertexBuffer->mapped = data;
        if (mappedVertices) *mappedVertices = (Vertex*)data;
    }
}

void vk_create_index_buffer(VulkanContext* context, uint16_t* indices, size_t indexCount, VulkanBuffer* indexBuffer, uint16_t** mappedIndices, VkMemoryPropertyFlags properties) {
    VkDeviceSize bufferSize = indexCount * sizeof(uint16_t);

    if (indices == NULL) {
        LOG_FATAL("indices == NULL - fatal error in vk_create_index_buffer");
    }

    bool isDeviceLocal = properties & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    if (isDeviceLocal) {
        VulkanBuffer stagingBuffer = {};
        vk_create_buffer(context, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &stagingBuffer);

        void* data;
        VK_CHECK(vkMapMemory(context->device, stagingBuffer.memory, 0, bufferSize, 0, &data));
        memcpy(data, indices, bufferSize);
        vkUnmapMemory(context->device, stagingBuffer.memory);

        vk_create_buffer(context, bufferSize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            properties, indexBuffer);

        vk_copy_buffer(context, stagingBuffer.handle, indexBuffer->handle, bufferSize);

        vkDestroyBuffer(context->device, stagingBuffer.handle, NULL);
        vkFreeMemory(context->device, stagingBuffer.memory, NULL);

        if (mappedIndices) *mappedIndices = NULL;
    } else {
        vk_create_buffer(context, bufferSize,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            properties, indexBuffer);

        void* data;
        VK_CHECK(vkMapMemory(context->device, indexBuffer->memory, 0, bufferSize, 0, &data));
        memcpy(data, indices, bufferSize);
        indexBuffer->mapped = data;
        if (mappedIndices) *mappedIndices = (uint16_t*)data;
    }
}

void vk_create_instance_buffer(VulkanContext* context, VulkanBuffer* instanceBuffer, uint32_t maxInstances) {
    VkDeviceSize bufferSize = maxInstances * sizeof(Vec4);

    Vec4* defaultOffsets = (Vec4*)calloc(maxInstances, sizeof(Vec4));

    vk_create_buffer(context, bufferSize,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        instanceBuffer);

    VK_CHECK(vkMapMemory(context->device, instanceBuffer->memory, 0, bufferSize, 0, &instanceBuffer->mapped));
    memcpy(instanceBuffer->mapped, defaultOffsets, bufferSize);

    free(defaultOffsets);
}

void vk_create_uniform_buffers(VulkanContext* context, VulkanBuffer* uniformBuffers) {
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vk_create_buffer(context, bufferSize,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &uniformBuffers[i]);

        VK_CHECK(vkMapMemory(context->device, uniformBuffers[i].memory, 0, bufferSize, 0, &uniformBuffers[i].mapped));
    }
}

void backend_createStorageBuffers2(VulkanContext* context, VulkanBuffer* storageBuffers) {
    VkDeviceSize bufferSize = sizeof(StorageBufferObject);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vk_create_buffer(context, bufferSize,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &storageBuffers[i]);

        VK_CHECK(vkMapMemory(context->device, storageBuffers[i].memory, 0, bufferSize, 0, &storageBuffers[i].mapped));
    }
}

void vk_update_vertex_buffer(VulkanContext* context, GraphicsObject* object, Vertex* newVertices, size_t newVertexCount, int newFormat) {
    size_t newSize = newVertexCount * sizeof(Vertex);
    size_t currentSize = object->vertexCount * sizeof(Vertex);

    if (object->vertexFormat != newFormat) {
        logMessage(LOG_LEVEL_WARNING,
            "Vertex format mismatch: expected format ID %d, got %d.",
            object->vertexFormat, newFormat);
    }

    if (newSize > currentSize) {
        vkDeviceWaitIdle(context->device);

        if (object->vertices != NULL) {
            vkUnmapMemory(context->device, object->vertexBuffer2.memory);
            object->vertices = NULL;
        }

        vkDestroyBuffer(context->device, object->vertexBuffer2.handle, NULL);
        vkFreeMemory(context->device, object->vertexBuffer2.memory, NULL);

        vk_create_vertex_buffer(context, newVertices, newVertexCount, &object->vertexBuffer2, &object->vertices,
                                     sizeof(Vertex),
                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        object->vertexCount = newVertexCount;

        logMessage(LOG_LEVEL_INFO, "Recreated vertex buffer with HOST_VISIBLE memory: %zu vertices", newVertexCount);
        return;
    }

    if (object->vertices != NULL) {
        memcpy(object->vertices, newVertices, newSize);
        object->vertexCount = newVertexCount;
        logMessage(LOG_LEVEL_DEBUG, "Updated existing vertex buffer: %zu vertices", newVertexCount);
    } else {
        logMessage(LOG_LEVEL_ERROR, "Vertex buffer not mapped - cannot update");
    }
}

void vk_update_index_buffer(VulkanContext* context, GraphicsObject* object, uint16_t* newIndices, size_t newIndexCount) {
    size_t newSize = newIndexCount * sizeof(uint16_t);
    size_t currentSize = object->indexCount * sizeof(uint16_t);

    if (newSize > currentSize) {
        vkDeviceWaitIdle(context->device);

        if (object->indices != NULL) {
            vkUnmapMemory(context->device, object->indexBuffer2.memory);
            object->indices = NULL;
        }

        vkDestroyBuffer(context->device, object->indexBuffer2.handle, NULL);
        vkFreeMemory(context->device, object->indexBuffer2.memory, NULL);

        vk_create_index_buffer(context, newIndices, newIndexCount, &object->indexBuffer2, &object->indices,
                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        object->indexCount = newIndexCount;

        logMessage(LOG_LEVEL_INFO, "Recreated index buffer with HOST_VISIBLE memory: %zu indices", newIndexCount);
        return;
    }

    if (object->indices != NULL) {
        memcpy(object->indices, newIndices, newSize);
        object->indexCount = newIndexCount;
        logMessage(LOG_LEVEL_DEBUG, "Updated existing index buffer: %zu indices", newIndexCount);
    } else {
        logMessage(LOG_LEVEL_ERROR, "Index buffer not mapped - cannot update");
    }
}

void vk_update_uniform_buffer(GraphicsObject* object) {
    uint32_t currentFrame = context.currentFrame;
    object->ubo.iResolution = (Vec2){(float)context.swapChainExtent.width, (float)context.swapChainExtent.height};
    object->ubo.iTime = sys_platform_get_elapsed_time();
    object->ubo.view = context.view;
    object->ubo.proj = context.proj;
    // object->ubo.model is already set by the caller
    
    memcpy(object->uniformBuffers2[currentFrame].mapped, &object->ubo, sizeof(object->ubo));
}

void vk_bind_vertex_buffer(GraphicsObject* object) {
    VkBuffer vertexBuffers[] = {object->vertexBuffer2.handle};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(context.commandBuffers[context.currentFrame], 
        0, 1, vertexBuffers, offsets);
}

void vk_bind_index_buffer(GraphicsObject* object) {
    vkCmdBindIndexBuffer(context.commandBuffers[context.currentFrame], 
        object->indexBuffer2.handle, 0, VK_INDEX_TYPE_UINT16);
}


void vk_cleanup_staging_buffer(VulkanContext* context) {

    if (context->stagingBuffer.mapped) {

        vkUnmapMemory(context->device, context->stagingBuffer.memory);

        context->stagingBuffer.mapped = NULL;

    }

    if (context->stagingBuffer.handle) {

        vkDestroyBuffer(context->device, context->stagingBuffer.handle, NULL);

        context->stagingBuffer.handle = VK_NULL_HANDLE;

    }

    if (context->stagingBuffer.memory) {

        vkFreeMemory(context->device, context->stagingBuffer.memory, NULL);

        context->stagingBuffer.memory = VK_NULL_HANDLE;

    }

    context->stagingBuffer.allocated_size = 0;

}



void vk_print_buffer(VulkanBuffer* buffer) {
    printf("=== VulkanBuffer Info ===\n");
    printf("  Handle        : %p\n",  (void*)buffer->handle);
    printf("  Memory        : %p\n",  (void*)buffer->memory);
    printf("  Requested     : %llu bytes\n", (unsigned long long)buffer->requested_size);
    printf("  Allocated     : %llu bytes\n", (unsigned long long)buffer->allocated_size);
    printf("  Wasted        : %llu bytes\n", (unsigned long long)(buffer->allocated_size - buffer->requested_size));
    printf("  Memory Type   : %u\n",  buffer->memory_type_index);
    printf("  Mapped        : %s\n",  buffer->mapped    ? "yes" : "no");
    printf("  Locked        : %s\n",  buffer->is_locked ? "yes" : "no");

    printf("  Usage         : ");
    if (buffer->usage & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)   printf("VERTEX ");
    if (buffer->usage & VK_BUFFER_USAGE_INDEX_BUFFER_BIT)    printf("INDEX ");
    if (buffer->usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)  printf("UNIFORM ");
    if (buffer->usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)  printf("STORAGE ");
    if (buffer->usage & VK_BUFFER_USAGE_TRANSFER_SRC_BIT)    printf("TRANSFER_SRC ");
    if (buffer->usage & VK_BUFFER_USAGE_TRANSFER_DST_BIT)    printf("TRANSFER_DST ");
    printf("\n");

    printf("  Properties    : ");
    if (buffer->properties & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)  printf("DEVICE_LOCAL ");
    if (buffer->properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)  printf("HOST_VISIBLE ");
    if (buffer->properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) printf("HOST_COHERENT ");
    printf("\n");
    printf("========================\n");
}
