#pragma once

#include <kata/ecs/registry.hpp>
#include <kata/input/input.hpp>

namespace kata {
class App {
public:
    App() = default;

    virtual void init() {};

    void process_raw_key_event(KeyEvent event);

    Registry& registry()
    {
        return m_registry;
    }

    InputHandler& input()
    {
        return m_input_handler;
    }

private:
    Registry m_registry {};
    InputHandler m_input_handler {};
};

void run(App& app);
}
