#include <kata/app/app.hpp>
#include <kata/ecs/system.hpp>
#include <kata/input/input.hpp>
#include <kata/render/render.hpp>
#include <kata/render/window.hpp>
#include <kata/resource/shader.hpp>
#include <spdlog/spdlog.h>

namespace kata {
void App::process_raw_key_event(KeyEvent event)
{
    m_input_handler.submit_key_event(event);
}

void App::handle_window_resize_event(Window::Size size)
{
    m_renderer.resize(size);
}

class GLFWInitGuard {
public:
    static void create()
    {
        static int result = glfwInit();
    }

    ~GLFWInitGuard()
    {
        glfwTerminate();
    }

private:
    GLFWInitGuard() = default;
};

void setup_glfw_callback_trampolines(App& app, Window& window)
{
    glfwSetWindowUserPointer(window.raw(), &app);
    glfwSetKeyCallback(window.raw(), [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        auto app = static_cast<App*>(glfwGetWindowUserPointer(window));
        app->process_raw_key_event(KeyEvent {
            .key = key,
            .scancode = scancode,
            .action = action,
            .mods = mods,
        });
    });

    glfwSetFramebufferSizeCallback(window.raw(), [](GLFWwindow* window, int width, int height) {
        if (width < 0 || height < 0) {
            panic(Error::with_message("negative value for width/height"));
        }

        auto app = static_cast<App*>(glfwGetWindowUserPointer(window));
        app->handle_window_resize_event(Window::Size {
            .width = uint32_t(width),
            .height = uint32_t(height),
        });
    });
}

void run(App& app)
{
    GLFWInitGuard::create();

    auto [compiler, compiler_err] = ShaderCompiler::create();
    if (compiler_err) {
        panic(compiler_err);
    }

    compiler.compile();

    auto [window, err] = Window::create(1280, 720, "kata");
    if (err) {
        panic(err);
    }

    auto [renderer, renderer_err] = kata::Renderer::create(std::move(window));
    if (renderer_err) {
        panic(renderer_err);
    }

    app.renderer() = std::move(renderer);

    app.init();

    setup_glfw_callback_trampolines(app, app.renderer().window());

    while (!app.renderer().window().should_close() && !app.input().is_key_pressed(Key::Escape)) {
        glfwPollEvents();

        app.renderer().render();
    }
}
}
