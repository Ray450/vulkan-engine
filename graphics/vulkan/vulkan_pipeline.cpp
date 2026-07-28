#include "vulkan_pipeline.h"

extern VulkanContext context;

void vk_create_graphics_pipeline(VulkanContext* context, VkDevice device,
    const char* vertShaderPath,
    const char* fragShaderPath,
    VkDescriptorSetLayout descriptorSetLayout,
    VkPipelineLayout* outPipelineLayout,
    VkPipeline* outGraphicsPipeline,
    VkRenderPass renderPass,
    int vertex_format,
    VkCullModeFlags cullMode,
    VkPrimitiveTopology topology,
    VkCompareOp depthOp) {
        //auto vertShaderCode = read_file(vertShaderPath);
        //auto fragShaderCode = read_file(fragShaderPath);


        size_t vertShaderSize, fragShaderSize;

    // Read the vertex shader file
    char* vertShaderCode = read_file(vertShaderPath, &vertShaderSize);
    if (!vertShaderCode) {
        LOG_FATAL("Failed to read vertex shader file: %s\n", vertShaderPath);
    }

    // Read the fragment shader file
    char* fragShaderCode = read_file(fragShaderPath, &fragShaderSize);
    if (!fragShaderCode) {
        LOG_FATAL("Failed to read fragment shader file: %s\n", fragShaderPath);
    }

        //VkShaderModule vertShaderModule = vk_create_shader_module(vertShaderCode);
        //VkShaderModule fragShaderModule = vk_create_shader_module(fragShaderCode);

        // Create shader modules
        VkShaderModule vertShaderModule = vk_create_shader_module(device, vertShaderCode, vertShaderSize);
        
        if (vertShaderModule == VK_NULL_HANDLE) {
            free(vertShaderCode);
            free(fragShaderCode);
            LOG_FATAL("Failed to create vertex shader module");
        }
        
        VkShaderModule fragShaderModule = vk_create_shader_module(device, fragShaderCode, fragShaderSize);

        if (fragShaderModule == VK_NULL_HANDLE) {
            vkDestroyShaderModule(device, vertShaderModule, NULL);
            free(vertShaderCode);
            free(fragShaderCode);
            LOG_FATAL("Failed to create fragment shader module");
        }


        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        //auto bindingDescription = getBindingDescription();
        //auto attributeDescriptions = getAttributeDescriptions();
        // VkVertexInputBindingDescription bindingDescription = getBindingDescription();
        
        VkVertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex); // Assuming `Vertex` is a user-defined struct
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        
        VkVertexInputAttributeDescription attributeDescriptions[4]; // Change to C-style array
        // getAttributeDescriptions(attributeDescriptions);


        uint32_t attributeCount = 0;
        
        //for(int i = 0; i < vertex_format + 2; i++) {
            
        
        if(vertex_format >= VERTEX_FORMAT_POS_COLOR) {
            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[0].offset = offsetof(Vertex, pos);
            attributeCount++;
            
            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
            attributeDescriptions[1].offset = offsetof(Vertex, color);
            attributeCount++;
        }
        
        if (vertex_format >= VERTEX_FORMAT_POS_COLOR_TEX) 
        {
            attributeDescriptions[2].binding = 0;
            attributeDescriptions[2].location = 2;
            attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[2].offset = offsetof(Vertex, texCoord);
            attributeCount++;
        }
        
        if (vertex_format == VERTEX_FORMAT_POS_COLOR_TEX_NORM) 
        {
            attributeDescriptions[3].binding = 0;
            attributeDescriptions[3].location = 3;
            attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[3].offset = offsetof(Vertex, normal);
            attributeCount++;
        }
        //printf("index: %d, enum: %d, format: %d, \n", i, VERTEX_FORMAT_POS_COLOR_TEX, vertex_format);
        //}
        
            
        
        /*if (vertex_format == 1) 
        {
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);


        
            attributeDescriptions[2].binding = 0;
            attributeDescriptions[2].location = 2;
            attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[2].offset = offsetof(Vertex, texCoord);
            
            attributeDescriptions[3].binding = 0;
            attributeDescriptions[3].location = 3;
            attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[3].offset = offsetof(Vertex, normal);
            
            attributeCount = 4; // Include the texCoord and normal attribute
        }*/

        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = attributeCount;
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions;

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = topology;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = cullMode;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = depthOp;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.stencilTestEnable = VK_FALSE;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;


        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        VkDynamicState dynamicStates[2] = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
        
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = 2;
        dynamicState.pDynamicStates = dynamicStates;

        VkPushConstantRange pushConstantRange = {};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;  // Available in vertex shader
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(PushConstantData);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        // printf("Size of PushConstantData: %zu bytes\n", sizeof(PushConstantData));


        VK_CHECK(vkCreatePipelineLayout(device, &pipelineLayoutInfo, NULL, outPipelineLayout));

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = *outPipelineLayout;
        pipelineInfo.renderPass = renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        VK_CHECK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, outGraphicsPipeline));

        context->totalPipelines++;

        vkDestroyShaderModule(device, fragShaderModule, NULL);
        vkDestroyShaderModule(device, vertShaderModule, NULL);
}

void vk_create_compute_pipeline(
    VkDevice device,
    const char* filename,
    VkDescriptorSetLayout computeDescriptorSetLayout,
    VkPipelineLayout* outComputePipelineLayout,
    VkPipeline* outComputePipeline
) {
    // Validate inputs
    if (!device || !filename || !computeDescriptorSetLayout || !outComputePipelineLayout || !outComputePipeline) {
        LOG_FATAL("Invalid parameters in backend_createComputePipeline: device=%p, filename=%p, "
                "computeDescriptorSetLayout=%p, outComputePipelineLayout=%p, outComputePipeline=%p\n",
                (void*)device, (void*)filename, (void*)computeDescriptorSetLayout,
                (void*)outComputePipelineLayout, (void*)outComputePipeline);
    }

    // Load shader code
    size_t compShaderSize;
    char* compShaderCode = read_file(filename, &compShaderSize);
    if (!compShaderCode) {
        logMessage(LOG_LEVEL_ERROR, "Failed to read compute shader file: %s\n", filename);
        return;
    }

    // Create shader module
    VkShaderModule computeShaderModule = vk_create_shader_module(device, compShaderCode, compShaderSize);
    free(compShaderCode);
    if (computeShaderModule == VK_NULL_HANDLE) {
        LOG_FATAL("Failed to create compute shader module for file: %s\n", filename);
    }

    // Define shader stage
    VkPipelineShaderStageCreateInfo computeShaderStageInfo = {};
    computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    computeShaderStageInfo.module = computeShaderModule;
    computeShaderStageInfo.pName = "main";

    // Define push constant range
    VkPushConstantRange pushConstant = {};
    pushConstant.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    pushConstant.offset = 0;
    pushConstant.size = sizeof(ComputePushConstantData);
    /*if (pushConstant.size != 64) {
        logMessage(LOG_LEVEL_ERROR, "Push constant size mismatch: expected 60 bytes, got %u bytes\n", pushConstant.size);
        vkDestroyShaderModule(device, computeShaderModule, NULL);
        return;
    }*/

    // Create pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &computeDescriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstant;

    VK_CHECK(vkCreatePipelineLayout(device, &pipelineLayoutInfo, NULL, outComputePipelineLayout));

    // Create compute pipeline
    VkComputePipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.stage = computeShaderStageInfo;
    pipelineInfo.layout = *outComputePipelineLayout;

    VK_CHECK(vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, outComputePipeline));

    // Clean up shader module
    vkDestroyShaderModule(device, computeShaderModule, NULL);

}



void vk_bind_pipeline(GraphicsObject* object) {
    vkCmdBindPipeline(context.commandBuffers[context.currentFrame], 
        VK_PIPELINE_BIND_POINT_GRAPHICS, object->graphicsPipeline);
}

void vk_push_constants(GraphicsObject* object) {
    vkCmdPushConstants(context.commandBuffers[context.currentFrame], 
        object->pipelineLayout, 
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
        0, sizeof(PushConstantData), &object->pushConstants);
}




void vk_cleanup_pipeline(VkDevice device, VkPipelineLayout* pipelineLayout, VkPipeline* graphicsPipeline) {
    vkDestroyPipeline(device, *graphicsPipeline, NULL);
    vkDestroyPipelineLayout(device, *pipelineLayout, NULL);

    if (*graphicsPipeline != VK_NULL_HANDLE) {
        *graphicsPipeline = VK_NULL_HANDLE;
    }

    if (*pipelineLayout != VK_NULL_HANDLE) {
        *pipelineLayout = VK_NULL_HANDLE;
    }
}
