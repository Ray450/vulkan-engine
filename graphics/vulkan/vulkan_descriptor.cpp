#include "vulkan_descriptor.h"

void vk_create_graphics_descriptor_set_layout(VkDevice device, VkDescriptorSetLayout* descriptorSetLayout) {
    // Create graphics descriptor set layout
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    // ADD THIS
    VkDescriptorSetLayoutBinding instanceLayoutBinding{};
    instanceLayoutBinding.binding = 2;
    instanceLayoutBinding.descriptorCount = 1;
    instanceLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    instanceLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding graphicsBindings[] = {
        uboLayoutBinding, 
        samplerLayoutBinding,
        instanceLayoutBinding  // ADD THIS
    };

    VkDescriptorSetLayoutCreateInfo graphicsLayoutInfo{};
    graphicsLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    graphicsLayoutInfo.bindingCount = 3;
    graphicsLayoutInfo.pBindings = graphicsBindings;

    VK_CHECK(vkCreateDescriptorSetLayout(device, &graphicsLayoutInfo, NULL, descriptorSetLayout));
}

void vk_create_compute_descriptor_set_layout(VkDevice device, VkDescriptorSetLayout* computeDescriptorSetLayout) {
    VkDescriptorSetLayoutBinding bindings[6] = {};

    for (int i = 0; i < 2; i++) {
        bindings[i].binding = i;
        bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        bindings[i].descriptorCount = 1;
        bindings[i].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 2;
    layoutInfo.pBindings = bindings;

    VK_CHECK(vkCreateDescriptorSetLayout(device, &layoutInfo, NULL, computeDescriptorSetLayout));
}

void vk_create_descriptor_set_layouts(VkDevice device, VkDescriptorSetLayout* graphicsDescriptorSetLayout, VkDescriptorSetLayout* computeDescriptorSetLayout) {
    vk_create_graphics_descriptor_set_layout(device, graphicsDescriptorSetLayout);
    vk_create_compute_descriptor_set_layout(device, computeDescriptorSetLayout);
}

void vk_create_graphics_descriptor_pool(VkDevice device, VkDescriptorPool* descriptorPool) {
    VkDescriptorPoolSize poolSizes[3];

    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = (uint32_t)(MAX_FRAMES_IN_FLIGHT);

    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = (uint32_t)(MAX_FRAMES_IN_FLIGHT);

    poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[2].descriptorCount = (uint32_t)(MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo poolInfo;
    memset(&poolInfo, 0, sizeof(poolInfo));
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 3;
    poolInfo.pPoolSizes = poolSizes;
    poolInfo.maxSets = (uint32_t)(MAX_FRAMES_IN_FLIGHT);

    VK_CHECK(vkCreateDescriptorPool(device, &poolInfo, NULL, descriptorPool));
}

void vk_create_compute_descriptor_pool(VkDevice device, VkDescriptorPool* descriptorPool) {
    // Validate inputs
    if (!device || !descriptorPool) {
        LOG_FATAL("Invalid parameters in vk_create_compute_descriptor_pool: device=%p, descriptorPool=%p\n",
                (void*)device, (void*)descriptorPool);
    }

    // Define pool sizes: 1 uniform buffer + 5 storage buffers per set
    VkDescriptorPoolSize poolSizes[2] = {
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * 2) }
    };

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 2;
    poolInfo.pPoolSizes = poolSizes;
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    VK_CHECK(vkCreateDescriptorPool(device, &poolInfo, NULL, descriptorPool));
}

void vk_create_graphics_descriptor_sets(
    VkDevice device,
    const VkBuffer* uniformBuffers,  // Pass by reference to avoid copying
    const VkDescriptorSetLayout descriptorSetLayout, // Pass descriptor set layout as reference
    VkDescriptorPool descriptorPool,
    const VkImageView textureImageView,  // Pass by reference for texture image view
    const VkSampler textureSampler,     // Pass by reference for texture sampler
    VkDescriptorSet* descriptorSets,  // Pass by reference to modify the descriptor sets
    VkBuffer instanceBuffer,      
    VkDeviceSize instanceBufferSize  

) {
    
    
    
    VkDescriptorSetLayout layouts[MAX_FRAMES_IN_FLIGHT];
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        layouts[i] = descriptorSetLayout;
    }
    
    
    VkDescriptorSetAllocateInfo allocInfo{};
    
    
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts;

    //descriptorSets->resize(MAX_FRAMES_IN_FLIGHT);
    VK_CHECK(vkAllocateDescriptorSets(device, &allocInfo, descriptorSets));

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkDescriptorBufferInfo instanceBufferInfo{};
        instanceBufferInfo.buffer = instanceBuffer;
        instanceBufferInfo.offset = 0;
        instanceBufferInfo.range  = instanceBufferSize;

        

        //std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
        
        VkWriteDescriptorSet descriptorWrites[3] = {};  // Array of 3 descriptor writes


        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[2].dstSet = descriptorSets[i];
        descriptorWrites[2].dstBinding = 2;
        descriptorWrites[2].dstArrayElement = 0;
        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrites[2].descriptorCount = 1;
        descriptorWrites[2].pBufferInfo = &instanceBufferInfo;

        
        if (textureImageView != NULL && textureSampler != NULL) {
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = textureImageView;
            imageInfo.sampler = textureSampler;
        
            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = descriptorSets[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &imageInfo;
            vkUpdateDescriptorSets(device, 3, descriptorWrites, 0, NULL);
        } else {
            VkWriteDescriptorSet writes[2] = {descriptorWrites[0], descriptorWrites[2]};

            vkUpdateDescriptorSets(device, 2, writes, 0, NULL);
        }

        
    }
}

//new one
void vk_create_compute_descriptor_sets(
    VulkanContext* context,
    VkDevice device,
    VkBuffer* buffers,      // flat array of all buffers [binding0, binding1, ..., bindingN]
    uint32_t bindingCount,  // how many bindings this shader uses
    VkDescriptorPool descriptorPool,
    VkDescriptorSet* descriptorSets,
    size_t setCount
) {
    VkDescriptorSetLayout* layouts = (VkDescriptorSetLayout*)malloc(setCount * sizeof(VkDescriptorSetLayout));
    for (size_t i = 0; i < setCount; i++)
        layouts[i] = context->computeDescriptorSetLayout;

    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = (uint32_t)setCount;
    allocInfo.pSetLayouts = layouts;

    VK_CHECK(vkAllocateDescriptorSets(device, &allocInfo, descriptorSets));
    free(layouts);

    for (size_t i = 0; i < setCount; i++) {
        VkDescriptorBufferInfo* bufferInfos = (VkDescriptorBufferInfo*)malloc(bindingCount * sizeof(VkDescriptorBufferInfo));
        VkWriteDescriptorSet*   writes      = (VkWriteDescriptorSet*)  malloc(bindingCount * sizeof(VkWriteDescriptorSet));

        for (uint32_t b = 0; b < bindingCount; b++) {
            bufferInfos[b] = {};
            bufferInfos[b].buffer = buffers[i * bindingCount + b];
            bufferInfos[b].offset = 0;
            bufferInfos[b].range  = VK_WHOLE_SIZE;

            writes[b] = {};
            writes[b].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writes[b].dstSet          = descriptorSets[i];
            writes[b].dstBinding      = b;
            writes[b].descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            writes[b].descriptorCount = 1;
            writes[b].pBufferInfo     = &bufferInfos[b];
        }

        vkUpdateDescriptorSets(device, bindingCount, writes, 0, NULL);

        free(bufferInfos);
        free(writes);
    }
}

void vk_bind_descriptor_sets(GraphicsObject* object) {
    uint32_t currentFrame = context.currentFrame;
    if (object->pipelineLayout == VK_NULL_HANDLE) {
        LOG_FATAL("backend_bindDescriptorSets: pipelineLayout is VK_NULL_HANDLE");
    }
    vkCmdBindDescriptorSets(context.commandBuffers[context.currentFrame], 
        VK_PIPELINE_BIND_POINT_GRAPHICS, object->pipelineLayout, 
        0, 1, &object->descriptorSets[currentFrame], 0, NULL);
}