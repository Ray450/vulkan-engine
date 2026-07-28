#include <stdlib.h>
#include <stdbool.h>
#include <stdbool.h>
#include <math.h>
#include <stdio.h>
#include <math.h> 
#include <time.h>
#include <limits.h>
#include "vulkan_graphics.h"


#include "vulkan_surface.h"
#include "vulkan_renderpass.h"

void init_graphics(int width, int height) {
    size_t imageCount = 0;
    LOG_INFO("Setting up graphics...");
    sys_create_window(&window.handle, width, height);

    context.window  = window.handle;

    vk_create_instance(&context.instance, enableValidationLayers, validationLayers, 1);
    vk_setup_debug_messenger(context.instance, &context.debugMessenger);
    vk_create_surface(context.instance, context.window, &context.surface);
    vk_pick_physical_device(&context.physicalDevice, &context.deviceLimits, context.instance, context.surface, deviceExtensions, 1, enableValidationLayers);
    vk_create_logical_device(&context.device, context.physicalDevice, context.surface, &context.graphicsQueue, &context.presentQueue, &context.computeQueue, deviceExtensions, 1, validationLayers, 1, enableValidationLayers);
    vk_create_swap_chain(&context.swapChain, context.swapChainImages, context.swapChainImageViews, &context.swapChainImageFormat, &context.swapChainExtent, context.swapChainFramebuffers, MAX_SWAPCHAIN_IMAGES, &imageCount, context.surface, context.physicalDevice, context.device, context.renderPass, context.window);
    vk_create_image_views(context.device, context.swapChainImages, context.swapChainImageViews, context.swapChainImageFormat, imageCount);
    vk_create_render_pass(context.device, context.physicalDevice, context.swapChainImageFormat, &context.renderPass);
    vk_create_depth_resources(context.device, context.physicalDevice, context.swapChainExtent, &context.depthImage, &context.depthImageMemory, &context.depthImageView);
    vk_create_framebuffers(context.device, context.renderPass, context.swapChainImageViews, context.depthImageView, context.swapChainFramebuffers, context.swapChainExtent, imageCount);

    context.swapChainImageCount = imageCount;

    vk_create_sync_objects(context.device, context.imageAvailableSemaphores, context.renderFinishedSemaphores, context.inFlightFences, context.computeImageAvailableSemaphores, context.computeFinishedSemaphores, context.computeInFlightFences, MAX_FRAMES_IN_FLIGHT);
    vk_create_descriptor_set_layouts(context.device, &context.descriptorSetLayout, &context.computeDescriptorSetLayout);
    vk_create_command_pool(context.device, context.physicalDevice, context.surface, &context.commandPool);

    vk_create_command_buffers(context.device, context.commandPool, context.commandBuffers, MAX_FRAMES_IN_FLIGHT);
    vk_init_staging_buffer(&context, 128 * 1024 * 1024);
    vk_create_single_time_fence(context.device, &context.singleTimeFence);
    
    vk_print_buffer(&context.stagingBuffer);


    LOG_INFO("Graphics initialized successfully.");
}

void create_graphics_pipeline(Pipeline* pipeline, const char* vertShaderPath, const char* fragShaderPath, int vertexFormat, VkCullModeFlags cullMode, VkPrimitiveTopology topology, VkCompareOp depthOp) {
    pipeline->vertShaderPath = vertShaderPath;
    pipeline->fragShaderPath = fragShaderPath;
    pipeline->vertexFormat = vertexFormat;
    pipeline->cullMode = cullMode;
    pipeline->topology = topology;

    LOG_INFO("createShapePipeline: descriptorSetLayout = %p, renderPass = %p", (void*)context.descriptorSetLayout, (void*)context.renderPass);

    if (context.descriptorSetLayout == VK_NULL_HANDLE) {
        LOG_WARN("Descriptor set layout not initialized, creating now.");
        vk_create_graphics_descriptor_set_layout(context.device, &context.descriptorSetLayout);
    }
    if (context.renderPass == VK_NULL_HANDLE) {
        LOG_WARN("Render pass not initialized, creating now.");
        vk_create_render_pass(context.device, context.physicalDevice, context.swapChainImageFormat, &context.renderPass);
    }

    vk_create_graphics_pipeline(&context, context.device, vertShaderPath, fragShaderPath, context.descriptorSetLayout,
                                   &pipeline->pipelineLayout, &pipeline->graphicsPipeline, context.renderPass,
                                   vertexFormat, cullMode, topology, depthOp);
}

Texture create_texture(VulkanContext* context, unsigned char* pixels, int width, int height, VkFormat format) {
    Texture tex = {0};
    vk_create_texture_from_pixels(context, &tex, pixels, width, height, format, VK_IMAGE_VIEW_TYPE_2D);
    vk_create_texture_image_view(context, &tex, format, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D);
    vk_create_texture_sampler(context, &tex);
    return tex;
}

void create_empty_texture(VulkanContext* context, Texture* texture, int width, int height, VkFormat format) {

    VkDeviceSize imageSize = width * height * 4;



    // Allocate blank pixel data (all zeros)

    unsigned char* blankPixels = (unsigned char*)calloc(imageSize, 1);

    if (!blankPixels) {

        logMessage(LOG_LEVEL_ERROR, "Failed to allocate blank pixel data");

        return;

    }



    // Create texture from blank pixels

    *texture = create_texture(context, blankPixels, width, height, format);

    free(blankPixels);


}

void copy_vertices(Vertex** dest, Vertex* src, size_t count) {
    *dest = (Vertex*)malloc(count * sizeof(Vertex));
    if (!(*dest)) {
        LOG_FATAL("Failed to allocate vertices");
        return;
    }
    memcpy(*dest, src, count * sizeof(Vertex));
}

void copy_indices(uint16_t** dest, uint16_t* src, size_t count) {
    *dest = (uint16_t*)malloc(count * sizeof(uint16_t));
    if (!(*dest)) {
        LOG_FATAL("Failed to allocate indices");
        return;
    }
    memcpy(*dest, src, count * sizeof(uint16_t));
}

void load_model(Vertex** model_vertices, int* model_vertexCount, uint16_t** model_indices, int* model_indexCount, const char* modelPath) {
    ModelData model = {0};

    if (!parse_obj(&model, modelPath)) {
        logMessage(LOG_LEVEL_ERROR, "Failed to load model: %s", modelPath);
        free(model.positions);
        free(model.texcoords);
        free(model.normals);
        free(model.indices);
        exit(EXIT_FAILURE);
    }

    *model_vertices = (Vertex*)malloc(model.indexCount * sizeof(Vertex));
    *model_indices = (uint16_t*)malloc(model.indexCount * sizeof(uint16_t));

    if (!*model_vertices || !*model_indices) {
        logMessage(LOG_LEVEL_ERROR, "Memory allocation failed for vertex and index data");
        free(model.positions);
        free(model.texcoords);
        free(model.normals);
        free(model.indices);
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < model.indexCount; i++) {
        (*model_vertices)[i].pos = model.positions[model.indices[i].v_idx];
        (*model_vertices)[i].texCoord = model.texcoords[model.indices[i].vt_idx];
        (*model_indices)[i] = (uint16_t)i;
    }

    *model_vertexCount = model.indexCount;
    *model_indexCount = model.indexCount;

    free(model.positions);
    free(model.texcoords);
    free(model.normals);
    free(model.indices);
}

GraphicsObject init_graphics_object(VulkanContext* context, Pipeline* pipeline, Texture texture, Vertex* vertices, uint16_t* indices, size_t vertexCount, size_t indexCount) {
    GraphicsObject obj;

    obj.pipelineLayout   = pipeline->pipelineLayout;
    obj.graphicsPipeline = pipeline->graphicsPipeline;
    obj.topology         = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    obj.vertexCount      = vertexCount;
    obj.indexCount       = indexCount;
    obj.vertexFormat     = VERTEX_FORMAT_POS_COLOR_TEX_NORM;
    obj.ubo.model        = createMat4(1.0f);
    obj.ubo.view         = createMat4(1.0f);
    obj.ubo.proj         = createMat4(1.0f);
    obj.pushConstants.color = vec4(1,1,1,1);
    obj.pushConstants.padding       = vec4(1,0,0,0);

    copy_vertices(&obj.vertices, vertices, vertexCount);
    copy_indices(&obj.indices, indices, indexCount);
    
    vk_create_vertex_buffer(context, obj.vertices, obj.vertexCount, &obj.vertexBuffer2, &obj.vertices2, sizeof(Vertex), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    // vk_print_buffer(&obj.vertexBuffer2);
    vk_create_index_buffer(context, obj.indices, obj.indexCount, &obj.indexBuffer2, &obj.indices2, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    //vk_print_buffer(&obj.indexBuffer2);

    obj.vertices = obj.vertices2;
    obj.indices  = obj.indices2;

    vk_create_uniform_buffers(context, obj.uniformBuffers2);
    vk_create_graphics_descriptor_pool(context->device, &obj.descriptorPool);

    obj.texture = texture;

    vk_create_instance_buffer(context, &obj.instanceBuffer2, 100);

    VkBuffer uniformHandles[MAX_FRAMES_IN_FLIGHT];
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        uniformHandles[i] = obj.uniformBuffers2[i].handle;
    }

    vk_create_graphics_descriptor_sets(context->device,
            uniformHandles,
            context->descriptorSetLayout,
            obj.descriptorPool,
            obj.texture.imageView,
            obj.texture.sampler,
            obj.descriptorSets,
            obj.instanceBuffer2.handle,
            100 * sizeof(Vec4));


    return obj;
}

GraphicsObject* create_graphics_object2(VulkanContext* context, Pipeline* pipeline, Texture texture, Vertex* vertices, uint16_t* indices, size_t vertexCount, size_t indexCount) {
    GraphicsObject* obj = (GraphicsObject*)calloc(1, sizeof(GraphicsObject));
    if (!obj) LOG_FATAL("GraphicsObject_create: failed to allocate");

    obj->pipelineLayout   = pipeline->pipelineLayout;
    obj->graphicsPipeline = pipeline->graphicsPipeline;
    obj->topology         = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    obj->vertexCount      = vertexCount;
    obj->indexCount       = indexCount;
    obj->vertexFormat     = VERTEX_FORMAT_POS_COLOR_TEX_NORM;
    obj->ubo.model        = createMat4(1.0f);
    obj->ubo.view         = createMat4(1.0f);
    obj->ubo.proj         = createMat4(1.0f);
    obj->pushConstants.color = vec4(1,1,1,1);
    obj->pushConstants.padding       = vec4(1,0,0,0);
    obj->texture = texture;

    copy_vertices(&obj->vertices, vertices, vertexCount);
    copy_indices(&obj->indices, indices, indexCount);

    vk_create_vertex_buffer(context, obj->vertices, obj->vertexCount, &obj->vertexBuffer2, &obj->vertices2, sizeof(Vertex), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    // vk_print_buffer(&obj->vertexBuffer2);
    vk_create_index_buffer(context, obj->indices, obj->indexCount, &obj->indexBuffer2, &obj->indices2, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    // vk_print_buffer(&obj->indexBuffer2);

    obj->vertices = obj->vertices2;
    obj->indices  = obj->indices2;
    
    vk_create_uniform_buffers(context, obj->uniformBuffers2);
    vk_create_graphics_descriptor_pool(context->device, &obj->descriptorPool);
    vk_create_instance_buffer(context, &obj->instanceBuffer2, 100);

    VkBuffer uniformHandles[MAX_FRAMES_IN_FLIGHT];
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        uniformHandles[i] = obj->uniformBuffers2[i].handle;
    }

    vk_create_graphics_descriptor_sets(context->device,
        uniformHandles,
        context->descriptorSetLayout,
        obj->descriptorPool,
        obj->texture.imageView,
        obj->texture.sampler,
        obj->descriptorSets,
        obj->instanceBuffer2.handle,
        100 * sizeof(Vec4));


    return obj;
}

// 2. The Wrapper (Allocates memory and uses the core setup)
GraphicsObject* create_graphics_object(VulkanContext* context, Pipeline* pipeline, Texture texture, Vertex* vertices, uint16_t* indices, size_t vertexCount, size_t indexCount) {
    GraphicsObject* obj = (GraphicsObject*)calloc(1, sizeof(GraphicsObject));
    if (!obj) LOG_FATAL("GraphicsObject_create: failed to allocate");

    // Call init and dereference the pointer to store the returned struct directly in the heap memory
    *obj = init_graphics_object(context, pipeline, texture, vertices, indices, vertexCount, indexCount);

    return obj;
}

void begin_frame(uint32_t imageIndex) {

    context.objectIndex = 0;

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(context.commandBuffers[context.currentFrame], &beginInfo) != VK_SUCCESS) {
        logMessage(LOG_LEVEL_ERROR, "failed to begin recording command buffer!");
        exit(EXIT_FAILURE);
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = context.renderPass;
    renderPassInfo.framebuffer = context.swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = context.swapChainExtent;


    

    VkClearValue clearValues[2] = {};
    clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};

    renderPassInfo.clearValueCount = 2;
    renderPassInfo.pClearValues = clearValues;

    vkCmdBeginRenderPass(context.commandBuffers[context.currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

struct Vec4 color;

void graphics_set_state(GraphicsObject* obj, struct mat4 finalTransform, struct Vec4 color, float mode) {
    // 1. Accept the exact matrix the user calculated
    obj->ubo.model = finalTransform;
    
    // 2. Sync with GPU
    vk_update_uniform_buffer(obj);
    
    // 3. Set global push constants
    obj->pushConstants.color = color;
    obj->pushConstants.padding = (struct Vec4){mode, 0.0f, 0.0f, 0.0f};
}

// Binds the graphics object to the GPU (Call this once before looping)
void graphics_bind(GraphicsObject* obj) {
    vk_update_uniform_buffer(obj);
    vk_bind_pipeline(obj);
    vk_bind_descriptor_sets(obj);
    vk_bind_vertex_buffer(obj);
    vk_bind_index_buffer(obj);
}

// Draws a specific shape from the buffer (Call this inside your loop)
void graphics_draw_range(GraphicsObject* obj, uint32_t instanceCount, 
                             uint32_t firstIndex, uint32_t indexCount, 
                             float xPos, float yPos) 
{
    // Push the dynamic position for this specific shape
    obj->pushConstants.offset = (Vec4){xPos, yPos, 0.0f, -1.0f};
    vk_push_constants(obj);
    
    // Draw just this chunk of the vertex buffer
    vk_draw(obj, indexCount, instanceCount, firstIndex, 0, 0);
}

void end_frame() {
    vkCmdEndRenderPass(context.commandBuffers[context.currentFrame]);

    if (vkEndCommandBuffer(context.commandBuffers[context.currentFrame]) != VK_SUCCESS) {
        logMessage(LOG_LEVEL_ERROR, "failed to record command buffer!");
        exit(EXIT_FAILURE);
    }
}

void vk_cleanup_uniform_buffers_and_memory2(VkDevice device, GraphicsObject* object) {

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (object->uniformBuffers2[i].handle != VK_NULL_HANDLE) {
            vkDestroyBuffer(device, object->uniformBuffers2[i].handle, NULL);
            object->uniformBuffers2[i].handle = VK_NULL_HANDLE;
        }
        if (object->uniformBuffers2[i].memory != VK_NULL_HANDLE) {
            vkFreeMemory(device, object->uniformBuffers2[i].memory, NULL);
            object->uniformBuffers2[i].memory = VK_NULL_HANDLE;
        }
    }

    if (object->descriptorPool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(device, object->descriptorPool, NULL);
        object->descriptorPool = VK_NULL_HANDLE;
    }

    if (object->indexBuffer2.handle != VK_NULL_HANDLE) {
        vkDestroyBuffer(device, object->indexBuffer2.handle, NULL);
        object->indexBuffer2.handle = VK_NULL_HANDLE;
    }
    if (object->indexBuffer2.memory != VK_NULL_HANDLE) {
        vkFreeMemory(device, object->indexBuffer2.memory, NULL);
        object->indexBuffer2.memory = VK_NULL_HANDLE;
    }

    if (object->vertexBuffer2.handle != VK_NULL_HANDLE) {
        vkDestroyBuffer(device, object->vertexBuffer2.handle, NULL);
        object->vertexBuffer2.handle = VK_NULL_HANDLE;
    }
    if (object->vertexBuffer2.memory != VK_NULL_HANDLE) {
        vkFreeMemory(device, object->vertexBuffer2.memory, NULL);
        object->vertexBuffer2.memory = VK_NULL_HANDLE;
    }
}

void destroy_shape(GraphicsObject* vertex) {

    if (vertex->graphicsPipeline != VK_NULL_HANDLE) {
        vertex->graphicsPipeline = VK_NULL_HANDLE;
    }

    if (vertex->pipelineLayout != VK_NULL_HANDLE) {
        vertex->pipelineLayout = VK_NULL_HANDLE;
    }
        
    vk_cleanup_uniform_buffers_and_memory2(context.device, vertex);
}

void cleanup_graphics() {        
    
    
    vk_cleanup_staging_buffer(&context);


    vk_cleanup_swap_chain();

    vkDestroyRenderPass(context.device, context.renderPass, NULL);
    vkDestroyDescriptorSetLayout(context.device, context.descriptorSetLayout, NULL);
    vkDestroyDescriptorSetLayout(context.device, context.computeDescriptorSetLayout, NULL);
    vkDestroyFence(context.device, context.singleTimeFence, NULL);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(context.device, context.renderFinishedSemaphores[i], NULL);
        vkDestroySemaphore(context.device, context.imageAvailableSemaphores[i], NULL);
        vkDestroyFence(context.device, context.inFlightFences[i], NULL);
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(context.device, context.computeImageAvailableSemaphores[i], NULL);
        vkDestroySemaphore(context.device, context.computeFinishedSemaphores[i], NULL);
        vkDestroyFence(context.device, context.computeInFlightFences[i], NULL);
    }

    vkDestroyCommandPool(context.device, context.commandPool, NULL);
    vkDestroyDevice(context.device, NULL);

    if (enableValidationLayers) {
        vk_destroy_debug_utils_messenger_ext(context.instance, context.debugMessenger, NULL);
    }

    vkDestroySurfaceKHR(context.instance, context.surface, NULL);
    vkDestroyInstance(context.instance, NULL);

    sys_destroy_window((void*)context.window);
}

void set_rendering_mode(VulkanContext* context, bool is3D) {
    float aspectRatio = (float)context->swapChainExtent.width / (float)context->swapChainExtent.height;

    if (is3D) {
        context->view = createLookAtMatrix(
            (struct Vec3){2.0f, 2.0f, 2.0f},
            (struct Vec3){0.0f, 0.0f, 0.0f},
            (struct Vec3){0.0f, 0.0f, 1.0f}
        );
        context->proj = createPerspectiveMatrix(
            degreesToRadians(45.0f),
            aspectRatio,
            0.1f,
            10.0f
        );
        context->proj = invertYAxis(context->proj);
    } else {
        context->view = createLookAtMatrix(
            (struct Vec3){0.0f, 0.0f, 1.0f},
            (struct Vec3){0.0f, 0.0f, 0.0f},
            (struct Vec3){0.0f, 1.0f, 0.0f}
        );
        context->proj = createOrthographicMatrix(
            -1.0f * aspectRatio, 1.0f * aspectRatio, -1.0f, 1.0f,
            1.0f, -1.0f
        );
        // context->proj.elements[0] *= -1.0f; // Flip x-axis if needed
        // context->proj = invertYAxis(context->proj);

    }
}

int get_graphics_width() {
    return vk_get_width();
}

int get_graphics_height() {
    return vk_get_height();
}