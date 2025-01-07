#include <GLFW/glfw3.h>
#include <kata/input/input.hpp>

namespace kata {
bool InputHandler::is_key_pressed(Key key) const
{
    Scancode scancode = glfwGetKeyScancode(int(key));

    return m_pressed_keys.contains(scancode);
}

void InputHandler::submit_key_event(KeyEvent event)
{
    switch (event.action) {
    case GLFW_PRESS:
        m_pressed_keys.insert(event.scancode);
        break;

    case GLFW_RELEASE:
        m_pressed_keys.erase(event.scancode);
        break;
    }
}
}
