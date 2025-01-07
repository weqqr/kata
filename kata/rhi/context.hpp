#pragma once

#include <kata/render/window.hpp>
#include <kata/rhi/command.hpp>
#include <string>
#include <vector>
#include <volk.h>

namespace kata {
struct SelectedPhysicalDevice {
    VkPhysicalDevice device { VK_NULL_HANDLE };
    std::string name {};
    VkSurfaceFormatKHR surface_format {};
    uint32_t queue_family { VK_QUEUE_FAMILY_IGNORED };
};

struct SwapchainFrame {
    VkImage swapchain_image { VK_NULL_HANDLE };
    VkImageView swapchain_image_view { VK_NULL_HANDLE };
    VkSemaphore acquire_semaphore { VK_NULL_HANDLE };
    GPUCommandList command_list {};
    uint64_t prev_progress { 0 };
};

struct QueueSync {
    VkSemaphore present_semaphore { VK_NULL_HANDLE };
    VkSemaphore timeline_semaphore { VK_NULL_HANDLE };
    VkSemaphore next_acquire_semaphore { VK_NULL_HANDLE };
    uint64_t progress { 0 };
};

class CurrentFrame {
    friend class GPUContext;

public:
    CurrentFrame() = default;

private:
    CurrentFrame(uint32_t index)
        : m_index(index)
    {
    }

    uint32_t m_index;
};

class GPUContext {
public:
    GPUContext() = default;

    static Result<GPUContext> with_window(Window const& window);

    ~GPUContext();

    GPUContext(GPUContext const&) = delete;
    GPUContext& operator=(GPUContext const&) = delete;

    GPUContext(GPUContext&& other)
    {
        *this = std::move(other);
    }

    GPUContext& operator=(GPUContext&& other)
    {
        std::swap(m_instance, other.m_instance);
        std::swap(m_debug_messenger, other.m_debug_messenger);
        std::swap(m_surface, other.m_surface);
        std::swap(m_physical_device, other.m_physical_device);
        std::swap(m_device, other.m_device);
        std::swap(m_queue, other.m_queue);
        std::swap(m_command_pool, other.m_command_pool);
        std::swap(m_swapchain, other.m_swapchain);
        std::swap(m_swapchain_frames, other.m_swapchain_frames);
        std::swap(m_queue_sync, other.m_queue_sync);

        return *this;
    }

    Result<CurrentFrame> begin_frame();
    void end_frame(CurrentFrame frame);

private:
    GPUContext(
        VkInstance instance,
        VkDebugUtilsMessengerEXT debug_messenger,
        VkSurfaceKHR surface,
        SelectedPhysicalDevice physical_device,
        VkDevice device,
        VkQueue queue,
        VkCommandPool command_pool,
        VkSwapchainKHR swapchain,
        std::vector<SwapchainFrame> swapchain_frames,
        QueueSync queue_sync)
        : m_instance(instance)
        , m_debug_messenger(debug_messenger)
        , m_surface(surface)
        , m_physical_device(physical_device)
        , m_device(device)
        , m_queue(queue)
        , m_command_pool(command_pool)
        , m_swapchain(swapchain)
        , m_swapchain_frames(std::move(swapchain_frames))
        , m_queue_sync(queue_sync)
    {
    }

    VkInstance m_instance { VK_NULL_HANDLE };
    VkDebugUtilsMessengerEXT m_debug_messenger { VK_NULL_HANDLE };
    VkSurfaceKHR m_surface { VK_NULL_HANDLE };
    SelectedPhysicalDevice m_physical_device {};
    VkDevice m_device { VK_NULL_HANDLE };
    VkQueue m_queue { VK_NULL_HANDLE };
    VkCommandPool m_command_pool {};
    VkSwapchainKHR m_swapchain { VK_NULL_HANDLE };
    std::vector<SwapchainFrame> m_swapchain_frames {};
    QueueSync m_queue_sync {};
};
}
