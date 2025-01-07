#pragma once

#include <kata/core/error.hpp>
#include <volk.h>

namespace kata {
class GPUCommandList {
    friend class GPUContext;

public:
    GPUCommandList() = default;
    ~GPUCommandList();

    static Result<GPUCommandList> create(VkDevice device, VkCommandPool pool);

    GPUCommandList(GPUCommandList const&) = delete;
    GPUCommandList& operator=(GPUCommandList const&) = delete;

    GPUCommandList(GPUCommandList&& other)
    {
        *this = std::move(other);
    }

    GPUCommandList& operator=(GPUCommandList&& other)
    {
        std::swap(m_device, other.m_device);
        std::swap(m_command_pool, other.m_command_pool);
        std::swap(m_buffer, other.m_buffer);

        return *this;
    }

private:
    GPUCommandList(VkDevice device, VkCommandPool pool, VkCommandBuffer buffer)
        : m_device(device)
        , m_command_pool(pool)
        , m_buffer(buffer)
    {
    }

    void reset();
    void begin();
    void finish();
    VkCommandBuffer raw() const;

    VkDevice m_device { VK_NULL_HANDLE };
    VkCommandPool m_command_pool { VK_NULL_HANDLE };
    VkCommandBuffer m_buffer { VK_NULL_HANDLE };
};
}
