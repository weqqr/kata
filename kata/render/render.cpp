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
}
