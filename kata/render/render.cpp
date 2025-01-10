#include <kata/render/render.hpp>
#include <kata/rhi/pipeline.hpp>

namespace kata {
Result<Renderer> Renderer::create(Window window, ShaderCompiler& compiler)
{
    auto [context, err] = GPUContext::with_window(window);
    if (err) {
        return err;
    }

    return Renderer(std::move(window), std::move(context), compiler);
}

Renderer::Renderer(Window window, GPUContext context, ShaderCompiler& compiler)
    : m_window(std::move(window))
    , m_context(std::move(context))
{
    auto [vertex_spirv, vertex_err] = compiler.compile_module_to_spirv("material", "vertex_main");
    if (vertex_err) {
        panic(vertex_err);
    }

    auto [fragment_spirv, fragment_err] = compiler.compile_module_to_spirv("material", "fragment_main");
    if (fragment_err) {
        panic(fragment_err);
    }

    auto [pipeline, err] = m_context.create_render_pipeline(GPURenderPipelineDesc {
        .vertex_spirv = vertex_spirv,
        .fragment_spirv = fragment_spirv,
    });

    if (err) {
        panic(err);
    }
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

void Renderer::resize(Window::Size size)
{
    m_context.resize_swapchain(size.width, size.height);
}
}
