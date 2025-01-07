#pragma once

#include <GLFW/glfw3.h>
#include <kata/core/error.hpp>

namespace kata {
class Window {
public:
    struct Size {
        uint32_t width;
        uint32_t height;
    };

    Window() = default;

    static Result<Window> create(int width, int height, char const* title);

    Window(Window const&) = delete;
    Window& operator=(Window const&) = delete;

    Window(Window&& other)
    {
        *this = std::move(other);
    }

    Window& operator=(Window&& other)
    {
        std::swap(m_window, other.m_window);

        return *this;
    }

    bool should_close() const;

    Size inner_size() const;

#ifdef _WIN32
    void* hwnd() const;
#endif

    GLFWwindow* raw() const;

private:
    explicit Window(GLFWwindow* window);

    GLFWwindow* m_window { nullptr };
};
}
