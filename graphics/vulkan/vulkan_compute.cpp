#include "vulkan_compute.h"

#include "vulkan_buffer.h"

#include "vulkan_command.h"

#include "vulkan_descriptor.h"

#include "vulkan_device.h"

#include "vulkan_graphics.h"

#include "vulkan_pipeline.h"

#include "vulkan_descriptor.h"

#include <string.h>

#include <stdio.h>



// ============================================================================

// CORE COMPUTE FUNCTIONS

// ============================================================================



void create_compute_Object(
    ComputeObject* object,

    const char* filename,

    size_t inputSize,

    size_t outputSize,

    uint32_t dispatchX,

    uint32_t dispatchY,

    uint32_t dispatchZ

) {





    // Initialize object

    memset(object, 0, sizeof(ComputeObject));

    object->pipelineLayout = VK_NULL_HANDLE;

    object->pipeline = VK_NULL_HANDLE;

    object->descriptorPool = VK_NULL_HANDLE;

    object->dispatchCountX = dispatchX;

    object->dispatchCountY = dispatchY;

    object->dispatchCountZ = dispatchZ;

    object->inputSize = inputSize;

    object->outputSize = outputSize;



    // Create compute pipeline

    vk_create_compute_pipeline(

        context.device,

        filename,

        context.computeDescriptorSetLayout,

        &object->pipelineLayout,

        &object->pipeline

    );



    // Create input and output buffers for each frame in flight

    // Create input and output buffers for each frame in flight

    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {

        // Input buffer (binding 0)

        vk_create_buffer(

            &context,

            inputSize,

            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,

            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,

            &object->inputBuffers[i]

        );

        vkMapMemory(

            context.device,

            object->inputBuffers[i].memory,

            0,

            inputSize,

            0,

            &object->inputBuffers[i].mapped

        );

        // Output buffer (binding 1)

        vk_create_buffer(

            &context,

            outputSize,

            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,

            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,

            &object->outputBuffers[i]

        );

        vkMapMemory(

            context.device,

            object->outputBuffers[i].memory,

            0,

            outputSize,

            0,

            &object->outputBuffers[i].mapped

        );

    }



    // Create descriptor pool and sets

    vk_create_compute_descriptor_pool(context.device, &object->descriptorPool);

    /*vk_create_compute_descriptor_sets(

        &context,

        context.device,

        object->inputBuffers,

        object->outputBuffers,

        object->descriptorPool,

        object->descriptorSets,

        MAX_FRAMES_IN_FLIGHT

    );*/



    

    // VkBuffer buffers[] = { &object->inputBuffers, &object->outputBuffers };



    VkBuffer buffers[MAX_FRAMES_IN_FLIGHT * 2];
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        buffers[i * 2 + 0] = object->inputBuffers[i].handle;
        buffers[i * 2 + 1] = object->outputBuffers[i].handle;
    }



    vk_create_compute_descriptor_sets(

    &context,

    context.device,

    buffers,

    2,

    object->descriptorPool,

    object->descriptorSets,

    MAX_FRAMES_IN_FLIGHT

);



    // vk_create_compute_descriptor_sets(context, context->device, buffers, 2, object->descriptorPool, object->descriptorSets, 1);



    // Create command buffers if not already created

    vk_create_compute_command_buffers(context.device, context.commandPool, context.computeCommandBuffers, MAX_FRAMES_IN_FLIGHT);



    object->isInitialized = true;

    printf("Compute object created: input=%zu bytes, output=%zu bytes, dispatch=(%u,%u,%u)\n",

           inputSize, outputSize, dispatchX, dispatchY, dispatchZ);

}



void update_compute_input(
    ComputeObject* object,

    const void* inputData,

    size_t dataSize,

    size_t offset

) {

    uint32_t frameIndex = context.currentFrame;

    if (!object || !object->inputBuffers[frameIndex].mapped) {

        fprintf(stderr, "Error: Invalid compute object or unmapped buffer\n");

        return;

    }



    if (offset + dataSize > object->inputSize) {

        fprintf(stderr, "Error: Data size (%zu + %zu) exceeds input buffer size (%zu)\n",

                offset, dataSize, object->inputSize);

        return;

    }



    // Copy data to mapped input buffer

    char* mappedPtr = (char*)object->inputBuffers[frameIndex].mapped;

    memcpy(mappedPtr + offset, inputData, dataSize);

}

void begin_compute() {
    
    VkCommandBuffer cmd = context.computeCommandBuffers[context.currentFrame];
    
    vkResetCommandBuffer(cmd, 0);

    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
    };

    if (vkBeginCommandBuffer(cmd, &begin_info) != VK_SUCCESS) {
        logMessage(LOG_LEVEL_ERROR, "failed to begin recording compute command buffer!");
        exit(EXIT_FAILURE);
    }
}

void end_compute() {
    VkCommandBuffer cmd = context.computeCommandBuffers[context.currentFrame];

    if (vkEndCommandBuffer(cmd) != VK_SUCCESS) {
        logMessage(LOG_LEVEL_ERROR, "failed to end compute command buffer!");
        exit(EXIT_FAILURE);
    }
}

void record_compute_dispatch(ComputeObject* object, const void* pushConstants, size_t pushConstantsSize) {

    uint32_t frameIndex = context.currentFrame;

    VkCommandBuffer cmd = context.computeCommandBuffers[context.currentFrame];


    // Bind compute pipeline

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, object->pipeline);



    // Bind descriptor sets

    vkCmdBindDescriptorSets(

        cmd,

        VK_PIPELINE_BIND_POINT_COMPUTE,

        object->pipelineLayout,

        0,

        1,

        &object->descriptorSets[frameIndex],

        0,

        nullptr

    );



    // Push constants (if provided)

    if (pushConstants && pushConstantsSize > 0) {

        vkCmdPushConstants(

            cmd,

            object->pipelineLayout,

            VK_SHADER_STAGE_COMPUTE_BIT,

            0,

            pushConstantsSize,

            pushConstants

        );

    }



    // Dispatch compute workgroups

    vkCmdDispatch(cmd, object->dispatchCountX, object->dispatchCountY, object->dispatchCountZ);

}

void submit_compute() {
    
    VkCommandBuffer cmd = context.computeCommandBuffers[context.currentFrame];

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;

    vkQueueSubmit(context.computeQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(context.computeQueue);
}




void* get_compute_output(ComputeObject* object) {

    uint32_t frameIndex = context.currentFrame;

    if (!object || !object->outputBuffers[frameIndex].mapped) {

        fprintf(stderr, "Error: Invalid compute object or unmapped output buffer\n");

        return nullptr;

    }



    // Wait for compute completion before reading

    wait_for_compute_completion(&context, frameIndex);



    return object->outputBuffers[frameIndex].mapped;

}



void* get_compute_input(ComputeObject* object) {

    uint32_t frameIndex = context.currentFrame;

    if (!object || !object->inputBuffers[frameIndex].mapped) {

        fprintf(stderr, "Error: Invalid compute object or unmapped input buffer\n");

        return nullptr;

    }



    return object->inputBuffers[frameIndex].mapped;

}



void wait_for_compute_completion(VulkanContext* context, uint32_t frameIndex) {

    VkResult result = vkWaitForFences(

        context->device,

        1,

        &context->computeInFlightFences[frameIndex],

        VK_TRUE,

        UINT64_MAX

    );



    if (result != VK_SUCCESS) {

        fprintf(stderr, "Error: Failed to wait for compute fence (frame %u)\n", frameIndex);

    }

}



void cleanup_compute_resources(

    VulkanContext* context,

    ComputeObject* object

) {

    if (!context || !object) {

        return;

    }



    // Wait for device to be idle before cleanup

    vkDeviceWaitIdle(context->device);



    // Destroy buffers for each frame

for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    // Input buffer
    if (object->inputBuffers[i].mapped) {
        vkUnmapMemory(context->device, object->inputBuffers[i].memory);
        object->inputBuffers[i].mapped = nullptr;
    }
    if (object->inputBuffers[i].handle != VK_NULL_HANDLE) {
        vkDestroyBuffer(context->device, object->inputBuffers[i].handle, nullptr);
        object->inputBuffers[i].handle = VK_NULL_HANDLE;
    }
    if (object->inputBuffers[i].memory != VK_NULL_HANDLE) {
        vkFreeMemory(context->device, object->inputBuffers[i].memory, nullptr);
        object->inputBuffers[i].memory = VK_NULL_HANDLE;
    }

    // Output buffer
    if (object->outputBuffers[i].mapped) {
        vkUnmapMemory(context->device, object->outputBuffers[i].memory);
        object->outputBuffers[i].mapped = nullptr;
    }
    if (object->outputBuffers[i].handle != VK_NULL_HANDLE) {
        vkDestroyBuffer(context->device, object->outputBuffers[i].handle, nullptr);
        object->outputBuffers[i].handle = VK_NULL_HANDLE;
    }
    if (object->outputBuffers[i].memory != VK_NULL_HANDLE) {
        vkFreeMemory(context->device, object->outputBuffers[i].memory, nullptr);
        object->outputBuffers[i].memory = VK_NULL_HANDLE;
    }
}



    // Destroy pipeline

    if (object->pipeline != VK_NULL_HANDLE) {

        vkDestroyPipeline(context->device, object->pipeline, nullptr);

        object->pipeline = VK_NULL_HANDLE;

    }



    // Destroy pipeline layout

    if (object->pipelineLayout != VK_NULL_HANDLE) {

        vkDestroyPipelineLayout(context->device, object->pipelineLayout, nullptr);

        object->pipelineLayout = VK_NULL_HANDLE;

    }



    // Destroy descriptor pool

    if (object->descriptorPool != VK_NULL_HANDLE) {

        vkDestroyDescriptorPool(context->device, object->descriptorPool, nullptr);

        object->descriptorPool = VK_NULL_HANDLE;

    }



    object->isInitialized = false;

    printf("Compute resources cleaned up\n");

}



// ============================================================================

// DATA EXPORT FUNCTIONS

// ============================================================================



void save_compute_output_to_csv(
    ComputeObject* object,

    const char* path,

    size_t numElements,

    size_t elementsPerRow

) {

        uint32_t frameIndex = context.currentFrame;


    if (!object || !path) {

        fprintf(stderr, "Error: Invalid parameters for CSV export\n");

        return;

    }

    wait_for_compute_completion(&context, frameIndex);



    // Get output data

    float* outputData = (float*)object->outputBuffers[frameIndex].mapped;

    if (!outputData) {

        fprintf(stderr, "Error: Output buffer not mapped\n");

        return;

    }



    // Calculate number of rows

    size_t numRows = (numElements + elementsPerRow - 1) / elementsPerRow;



    // Write to CSV

    write_float_array_to_csv(path, outputData, numRows, elementsPerRow, true);



    printf("Saved %zu elements (%zu rows x %zu cols) to %s\n",

           numElements, numRows, elementsPerRow, path);

}







// ============================================================================

// DEBUG/UTILITY FUNCTIONS

// ============================================================================



void print_compute_buffer(

    ComputeObject* object,

    const char* label,

    const float* data,

    uint32_t numElements

) {

    if (!object || !label || !data) {

        return;

    }



    printf("%s (first %u elements): ", label, numElements);

    for (uint32_t i = 0; i < numElements; i++) {

        printf("%.2f", data[i]);

        if (i < numElements - 1) printf(", ");

    }

    printf("\n");

}

