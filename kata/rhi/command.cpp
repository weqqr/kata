#include <kata/rhi/command.hpp>
#include <vector>

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

void GPUCommandList::begin_rendering(RenderingPassDescriptor const& desc)
{
    VkOffset2D offset {
        .x = desc.rect.x,
        .y = desc.rect.y,
    };

    VkExtent2D extent {
        .width = desc.rect.width,
        .height = desc.rect.height,
    };

    VkRect2D render_area {
        .offset = offset,
        .extent = extent,
    };

    VkClearValue clear_value {};
    clear_value.color.float32[0] = 1.0;
    clear_value.color.float32[1] = 0.4;
    clear_value.color.float32[2] = 0.2;
    clear_value.color.float32[3] = 1.0;

    std::vector<VkRenderingAttachmentInfo> color_attachments {};

    for (auto const& attachment : desc.color_attachments) {
        VkRenderingAttachmentInfo color_attachment {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = attachment.image_view,
            .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
            .resolveMode = VK_RESOLVE_MODE_NONE,
            .resolveImageView = VK_NULL_HANDLE,
            .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = clear_value,
        };

        color_attachments.push_back(color_attachment);
    }

    VkRenderingInfo rendering_info {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = render_area,
        .layerCount = 1,
        .viewMask = 0,
        .colorAttachmentCount = uint32_t(color_attachments.size()),
        .pColorAttachments = color_attachments.data(),
        .pDepthAttachment = nullptr, // FIXME: use depth attachment specified in desc
        .pStencilAttachment = nullptr,
    };

    vkCmdBeginRendering(m_buffer, &rendering_info);
}

void GPUCommandList::end_rendering()
{
    vkCmdEndRendering(m_buffer);
}

void GPUCommandList::transition_texture_layout(TextureView texture_view, VkImageLayout target)
{
    VkImageMemoryBarrier2 image_barrier {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = 0,
        .srcAccessMask = 0,
        .dstStageMask = 0,
        .dstAccessMask = 0,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = target,
        .srcQueueFamilyIndex = 0,
        .dstQueueFamilyIndex = 0,
        .image = texture_view.image,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };

    VkDependencyInfo dependency_info {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .memoryBarrierCount = 0,
        .pMemoryBarriers = nullptr,
        .bufferMemoryBarrierCount = 0,
        .pBufferMemoryBarriers = nullptr,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &image_barrier,
    };

    vkCmdPipelineBarrier2(m_buffer, &dependency_info);
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
