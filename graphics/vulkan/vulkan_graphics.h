#ifndef VULKAN_BACKEND_H
#define VULKAN_BACKEND_H

#include "vulkan_utils.h"
#include "vulkan_types.h"
#include "sys.h"
#include "vulkan_surface.h"
#include "vulkan_device.h"
#include "vulkan_swapchain.h"
#include "vulkan_sync.h"
#include "vulkan_renderpass.h"
#include "vulkan_descriptor.h"
#include "vulkan_command.h"
#include "vulkan_pipeline.h"
#include "vulkan_buffer.h"
#include "vulkan_image.h"

// #define DEBUG
#include "../logger.h"
#include "../graphics/math/matrix.h"

#include "file_utils.h"
#include "vulkan_compute.h"

typedef enum {
    RENDER_MODE_VERTEX_COLOR = 0,  // Default: Use vertex color
    RENDER_MODE_COLOR = 1,    // Use pushData.color
    RENDER_MODE_TEXTURE = 2,  // Use texture sampling
    MODE_COUNT
} RenderMode;

void init_graphics(int width, int height);
void create_graphics_pipeline(Pipeline* pipeline, const char* vertShaderPath, const char* fragShaderPath, int vertexFormat, VkCullModeFlags cullMode, VkPrimitiveTopology topology, VkCompareOp depthOp);
Texture create_texture(VulkanContext* context, unsigned char* pixels, int width, int height, VkFormat format);
void create_empty_texture(VulkanContext* context, Texture* texture, int width, int height, VkFormat format);
void copy_vertices(Vertex** dest, Vertex* src, size_t count);
void copy_indices(uint16_t** dest, uint16_t* src, size_t count);
void load_model(Vertex** model_vertices, int* model_vertexCount, uint16_t** model_indices, int* model_indexCount, const char* modelPath);
GraphicsObject* create_graphics_object(VulkanContext* context, Pipeline* pipeline, Texture texture, Vertex* vertices, uint16_t* indices, size_t vertexCount, size_t indexCount);
GraphicsObject init_graphics_object(VulkanContext* context, Pipeline* pipeline, Texture texture, Vertex* vertices, uint16_t* indices, size_t vertexCount, size_t indexCount);

void begin_frame(uint32_t imageIndex);
// Binds the graphics object to the GPU (Call this once before looping)
void graphics_bind(GraphicsObject* obj);

// Draws a specific shape from the buffer (Call this inside your loop)
void graphics_draw_range(GraphicsObject* obj, uint32_t instanceCount, 
                             uint32_t firstIndex, uint32_t indexCount, 
                             float xPos, float yPos);

// In static_polygon_batch.c
void graphics_set_state(GraphicsObject* obj, struct mat4 finalTransform, struct Vec4 color, float mode);
void end_frame();

void set_rendering_mode(VulkanContext* context, bool is3D);

int get_graphics_width();

int get_graphics_height();

void cleanup_uniform_buffers_and_memory(VkDevice device, GraphicsObject* object);


void destroy_shape(GraphicsObject* vertex);

void cleanup_graphics();

#endif
