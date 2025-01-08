#pragma once

#include <kata/core/error.hpp>
#include <span>
#include <volk.h>

namespace kata {
struct TextureView {
    VkImageView image_view { VK_NULL_HANDLE };
    VkImage image { VK_NULL_HANDLE };
};

struct Rect2D {
    int32_t x {};
    int32_t y {};
    uint32_t width {};
    uint32_t height {};
};

struct RenderingPassDescriptor {
    Rect2D rect {};
    std::span<TextureView> color_attachments {};
    TextureView depth_attachment {};
};

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

    void begin_rendering(RenderingPassDescriptor const& descriptor);
    void end_rendering();

    void transition_texture_layout(TextureView texture_view, VkImageLayout target);

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
