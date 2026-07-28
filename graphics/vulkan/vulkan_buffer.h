#ifndef VULKAN_BUFFER_H

#define VULKAN_BUFFER_H

#include "vulkan_types.h"

#include "vulkan_utils.h"

void vk_create_buffer(VulkanContext* context, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VulkanBuffer* outBuffer);

void vk_copy_buffer(VulkanContext* context, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

void vk_init_staging_buffer(VulkanContext* context, VkDeviceSize maxSize); 

void vk_create_vertex_buffer(VulkanContext* context, Vertex* vertices, size_t vertexCount, VulkanBuffer* vertexBuffer, Vertex** mappedVertices, size_t vertexSize, VkMemoryPropertyFlags properties);

void vk_create_index_buffer(VulkanContext* context, uint16_t* indices, size_t indexCount, VulkanBuffer* indexBuffer, uint16_t** mappedIndices, VkMemoryPropertyFlags properties);

void vk_create_storage_buffers(VulkanContext* context, VkBuffer* storageBuffers, VkDeviceMemory* storageBuffersMemory, void** storageBuffersMapped);

void vk_create_uniform_buffers(VulkanContext* context, VulkanBuffer* uniformBuffers);

void vk_create_instance_buffer(VulkanContext* context, VulkanBuffer* instanceBuffer, uint32_t maxInstances);

void vk_update_vertex_buffer(VulkanContext* context, GraphicsObject* object, Vertex* newVertices, size_t newVertexCount, int newFormat);

void vk_update_index_buffer(VulkanContext* context, GraphicsObject* object, uint16_t* newIndices, size_t newIndexCount);

void vk_update_uniform_buffer(GraphicsObject* object);

void vk_bind_vertex_buffer(GraphicsObject* object);

void vk_bind_index_buffer(GraphicsObject* object);

void vk_print_buffer(VulkanBuffer* buffer);

void vk_cleanup_staging_buffer(VulkanContext* context);

void vk_cleanup_uniform_buffers_and_memory(VkDevice device, GraphicsObject* object);

#endif // VULKAN_BUFFER_H