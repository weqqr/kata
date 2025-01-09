#pragma once

#include <kata/ecs/registry.hpp>
#include <kata/input/input.hpp>
#include <kata/render/render.hpp>

namespace kata {
class App {
public:
    App() = default;

    virtual void init() {};

    void process_raw_key_event(KeyEvent event);
    void handle_window_resize_event(Window::Size size);

    Registry& registry()
    {
        return m_registry;
    }

    InputHandler& input()
    {
        return m_input_handler;
    }

    Renderer& renderer()
    {
        return m_renderer;
    }

private:
    // FIXME: move these out of App (AppContainer? or into registry)
    Registry m_registry {};
    InputHandler m_input_handler {};
    Renderer m_renderer {};
};

void run(App& app);
}
