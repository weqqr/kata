#pragma once

#include <kata/rhi/context.hpp>
#include <kata/resource/shader.hpp>
#include <spdlog/spdlog.h>

namespace kata {
class Renderer {
public:
    Renderer() = default;

    Renderer(Renderer&&) = default;
    Renderer& operator=(Renderer&&) = default;

    static Result<Renderer> create(Window window, ShaderCompiler& compiler);

    Window& window()
    {
        return m_window;
    }

    void render();

    void resize(Window::Size size);

private:
    Renderer(Window, GPUContext, ShaderCompiler&);

    Window m_window {};
    GPUContext m_context {};
};
}
