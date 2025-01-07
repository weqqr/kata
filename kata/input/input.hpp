#pragma once

#include <GLFW/glfw3.h>
#include <kata/input/key.hpp>
#include <kata/render/window.hpp>
#include <unordered_set>

namespace kata {
using Scancode = int;

struct KeyEvent {
    int key;
    int scancode;
    int action;
    int mods;
};

class InputHandler {
public:
    InputHandler() = default;

    InputHandler(InputHandler const&) = delete;
    InputHandler& operator=(InputHandler const&) = delete;

    InputHandler(InputHandler&& other) = default;
    InputHandler& operator=(InputHandler&& other) = default;

    bool is_key_pressed(Key key) const;
    void submit_key_event(KeyEvent event);

    void events_in_this_frame();

    void flush_events();

private:
    std::unordered_set<Scancode> m_pressed_keys {};
};
}
