#ifndef VULKAN_COMPUTE_H
#define VULKAN_COMPUTE_H

#include "vulkan_utils.h"
#include "vulkan_types.h"

// ============================================================================
// CORE COMPUTE FUNCTIONS - Generalized for any compute task
// ============================================================================

void create_compute_Object(ComputeObject* object, const char* filename, size_t inputSize, size_t outputSize, uint32_t dispatchX, uint32_t dispatchY, uint32_t dispatchZ);

void update_compute_input(ComputeObject* object, const void* inputData, size_t dataSize, size_t offset);

void begin_compute();

void end_compute();

void record_compute_dispatch(ComputeObject* object, const void* pushConstants, size_t pushConstantsSize);

void submit_compute();

void* get_compute_output(ComputeObject* object);

void* get_compute_input(ComputeObject* object);

void wait_for_compute_completion(VulkanContext* context, uint32_t frameIndex);

void cleanup_compute_resources(VulkanContext* context, ComputeObject* object);

void save_compute_output_to_csv(ComputeObject* object, const char* path, size_t numElements, size_t elementsPerRow);



void print_compute_buffer(ComputeObject* object, const char* label, const float* data, uint32_t numElements);

#endif // VULKAN_COMPUTE_H