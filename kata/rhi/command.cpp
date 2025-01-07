#include <kata/rhi/command.hpp>

namespace kata {
Result<GPUCommandList> GPUCommandList::create(VkDevice device, VkCommandPool pool)
{
    VkCommandBufferAllocateInfo allocate_info {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    VkCommandBuffer buffer { VK_NULL_HANDLE };
    auto result = vkAllocateCommandBuffers(device, &allocate_info, &buffer);
    if (result != VK_SUCCESS) {
        return Error::with_message("unable to allocate command buffer");
    }

    return GPUCommandList(device, pool, buffer);
}

GPUCommandList::~GPUCommandList()
{
    if (m_device) {
        vkFreeCommandBuffers(m_device, m_command_pool, 1, &m_buffer);
    }
}

void GPUCommandList::begin()
{
    VkCommandBufferBeginInfo begin_info {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    };

    vkBeginCommandBuffer(m_buffer, &begin_info);
}

void GPUCommandList::reset()
{
    vkResetCommandBuffer(m_buffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
}

void GPUCommandList::finish()
{
    vkEndCommandBuffer(m_buffer);
}

VkCommandBuffer GPUCommandList::raw() const
{
    return m_buffer;
}
}
