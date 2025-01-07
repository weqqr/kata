#pragma once

#include <kata/rhi/context.hpp>
#include <spdlog/spdlog.h>

namespace kata {
class Renderer {
public:
    Renderer() = default;

    Renderer(Renderer&&) = default;
    Renderer& operator=(Renderer&&) = default;

    static Result<Renderer> create(Window window);

    Window& window()
    {
        return m_window;
    }

    void render()
    {
        auto [frame, err] = m_context.begin_frame();
        if (err) {
            spdlog::error("begin frame error: {}", err.text());
        }

        m_context.end_frame(frame);
    }

private:
    Renderer(Window, GPUContext);

    Window m_window {};
    GPUContext m_context {};
};
}
