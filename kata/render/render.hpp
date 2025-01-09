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

    void render();

    void resize(Window::Size size);

private:
    Renderer(Window, GPUContext);

    Window m_window {};
    GPUContext m_context {};
};
}
