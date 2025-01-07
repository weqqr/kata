#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <assert.h>
#include <kata/core/error.hpp>
#include <kata/render/window.hpp>

namespace kata {
Window::Window(GLFWwindow* window)
    : m_window(window)
{
}

Result<Window> Window::create(int width, int height, char const* title)
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    auto window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (window == nullptr) {
        return Error::with_message("failed to initialize window");
    }

    return Window(window);
}

bool Window::should_close() const
{
    return glfwWindowShouldClose(m_window);
}

Window::Size Window::inner_size() const
{
    int width, height;
    glfwGetFramebufferSize(m_window, &width, &height);

    assert(width >= 0 && height >= 0);

    return Size {
        .width = uint32_t(width),
        .height = uint32_t(height),
    };
}

#ifdef _WIN32
void* Window::hwnd() const
{
    return glfwGetWin32Window(m_window);
}
#endif

GLFWwindow* Window::raw() const
{
    return m_window;
}

}
