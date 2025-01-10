#pragma once

#include <kata/core/error.hpp>
#include <span>
#include <volk.h>

namespace kata {
struct GPURenderPipelineDesc {
    std::span<uint32_t> vertex_spirv;
    std::span<uint32_t> fragment_spirv;
    std::span<VkFormat> color_attachments {};
    VkFormat depth_attachment {};
};

class GPURenderPipeline {
public:
    GPURenderPipeline() = default;
    ~GPURenderPipeline();

    GPURenderPipeline(GPURenderPipeline const&) = delete;
    GPURenderPipeline& operator=(GPURenderPipeline const&) = delete;

    GPURenderPipeline(GPURenderPipeline&& other)
    {
        *this = std::move(other);
    }

    GPURenderPipeline& operator=(GPURenderPipeline&& other)
    {
        std::swap(m_device, other.m_device);
        std::swap(m_pipeline_layout, other.m_pipeline_layout);
        std::swap(m_pipeline, other.m_pipeline);

        return *this;
    }

    static Result<GPURenderPipeline> create(VkDevice device, GPURenderPipelineDesc desc);

private:
    GPURenderPipeline(VkDevice device, VkPipelineLayout pipeline_layout, VkPipeline pipeline)
        : m_device(device)
        , m_pipeline_layout(pipeline_layout)
        , m_pipeline(pipeline)
    {
    }

    VkDevice m_device { VK_NULL_HANDLE };
    VkPipelineLayout m_pipeline_layout { VK_NULL_HANDLE };
    VkPipeline m_pipeline { VK_NULL_HANDLE };
};
}
