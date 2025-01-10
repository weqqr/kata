#include <array>
#include <kata/rhi/pipeline.hpp>
#include <span>

namespace kata {
static Result<VkShaderModule> create_shader_module(VkDevice device, std::span<uint32_t>& spirv)
{
    VkShaderModuleCreateInfo create_info {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = spirv.size() * sizeof(uint32_t),
        .pCode = spirv.data(),
    };

    VkShaderModule shader_module = VK_NULL_HANDLE;
    auto result = vkCreateShaderModule(device, &create_info, nullptr, &shader_module);
    if (result != VK_SUCCESS) {
        return Error::with_message("unable to build shader module");
    }

    return shader_module;
}

static Result<VkPipelineLayout> create_pipeline_layout(VkDevice device)
{
    VkPipelineLayoutCreateInfo create_info {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 0,
        .pSetLayouts = nullptr,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = nullptr,
    };

    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
    auto result = vkCreatePipelineLayout(device, &create_info, nullptr, &pipeline_layout);
    if (result != VK_SUCCESS) {
        return Error::with_message("unable to create pipeline layout");
    }

    return pipeline_layout;
}

Result<GPURenderPipeline> GPURenderPipeline::create(VkDevice device, GPURenderPipelineDesc desc)
{
    if (desc.fragment_spirv.size() == 0 || desc.vertex_spirv.size() == 0) {
        return Error::with_message("unable to build render pipeline with empty shader bytecode");
    }

    //
    // Pipeline stages
    //
    VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 0,
        .pVertexBindingDescriptions = nullptr,
        .vertexAttributeDescriptionCount = 0, // FIXME
        .pVertexAttributeDescriptions = nullptr,
    };

    VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = false,
    };

    VkPipelineRasterizationStateCreateInfo rasterization_state_create_info {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_NONE,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0,
        .depthBiasClamp = 0.0,
        .depthBiasSlopeFactor = 0.0,
        .lineWidth = 1,
    };

    VkPipelineColorBlendStateCreateInfo color_blend_state_create_info {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_SET,
    };

    //
    // Shader stages
    //
    auto [vertex_module, vertex_err] = create_shader_module(device, desc.vertex_spirv)
        OR_RETURN(vertex_err);

    auto [fragment_module, fragment_err] = create_shader_module(device, desc.fragment_spirv)
        OR_RETURN(fragment_err);

    VkPipelineShaderStageCreateInfo vertex_shader_stage_create_info {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vertex_module,
        .pName = "vertex_main",
        .pSpecializationInfo = nullptr,
    };

    VkPipelineShaderStageCreateInfo fragment_shader_stage_create_info {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = fragment_module,
        .pName = "fragment_main",
        .pSpecializationInfo = nullptr,
    };

    std::array<VkPipelineShaderStageCreateInfo, 2> stages {
        vertex_shader_stage_create_info,
        fragment_shader_stage_create_info,
    };

    //
    // Layout
    //
    auto [pipeline_layout, layout_err] = create_pipeline_layout(device)
        OR_RETURN(layout_err);

    //
    // Pipeline
    //
    VkPipelineRenderingCreateInfo rendering_create_info {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .viewMask = 0,
        .colorAttachmentCount = uint32_t(desc.color_attachments.size()),
        .pColorAttachmentFormats = desc.color_attachments.data(),
        .depthAttachmentFormat = desc.depth_attachment,
        .stencilAttachmentFormat = desc.depth_attachment,
    };

    VkGraphicsPipelineCreateInfo create_info {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = &rendering_create_info,
        .stageCount = uint32_t(stages.size()),
        .pStages = stages.data(),
        .pVertexInputState = &vertex_input_state_create_info,
        .pInputAssemblyState = &input_assembly_state_create_info,
        .pTessellationState = nullptr,
        .pViewportState = nullptr,
        .pRasterizationState = &rasterization_state_create_info,
        .pMultisampleState = nullptr,
        .pDepthStencilState = nullptr,
        .pColorBlendState = &color_blend_state_create_info,
        .pDynamicState = nullptr,
        .layout = pipeline_layout,
        .renderPass = VK_NULL_HANDLE,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = 0,
    };

    VkPipeline pipeline = VK_NULL_HANDLE;
    auto result = vkCreateGraphicsPipelines(device, nullptr, 1, &create_info, nullptr, &pipeline);
    if (result == VK_SUCCESS) {
        return Error::with_message("unable to create render pipeline");
    }

    return GPURenderPipeline();
}

GPURenderPipeline::~GPURenderPipeline()
{
    if (!m_device) {
        return;
    }

    vkDestroyPipeline(m_device, m_pipeline, nullptr);
    vkDestroyPipelineLayout(m_device, m_pipeline_layout, nullptr);
}
}
