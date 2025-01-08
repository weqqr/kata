#include <kata/render/render.hpp>

namespace kata {
Result<Renderer> Renderer::create(Window window)
{
    auto [context, err] = GPUContext::with_window(window);
    if (err) {
        return err;
    }

    return Renderer(std::move(window), std::move(context));
}

Renderer::Renderer(Window window, GPUContext context)
    : m_window(std::move(window))
    , m_context(std::move(context))
{
}

void Renderer::render()
{
    auto [frame, err] = m_context.begin_frame();
    if (err) {
        spdlog::error("begin frame error: {}", err.text());
        panic(err);
    }

    auto& cmd = m_context.get_command_list_for_frame(frame);

    auto size = m_window.inner_size();
    cmd.transition_texture_layout(m_context.get_texture_view_for_frame(frame), VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL);

    std::vector<TextureView> color_attachments {
        m_context.get_texture_view_for_frame(frame),
    };

    cmd.begin_rendering(RenderingPassDescriptor {
        .rect = Rect2D {
            .x = 0,
            .y = 0,
            .width = size.width,
            .height = size.height,
        },
        .color_attachments = color_attachments,
    });

    cmd.end_rendering();

    m_context.end_frame(frame);
}
}
