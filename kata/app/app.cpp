#include <kata/app/app.hpp>
#include <kata/ecs/system.hpp>
#include <kata/render/render.hpp>
#include <kata/render/window.hpp>
#include <kata/input/input.hpp>
#include <spdlog/spdlog.h>

namespace kata {
void App::process_raw_key_event(KeyEvent event) {
    m_input_handler.submit_key_event(event);
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

void run(App& app)
{
    GLFWInitGuard::create();

    auto [window, err] = Window::create(1280, 720, "kata");
    if (err) {
        spdlog::error(err.text());
        exit(1);
    }

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

    auto [renderer, renderer_err] = kata::Renderer::create(std::move(window));
    if (renderer_err) {
        spdlog::error(renderer_err.text());
        exit(1);
    }

    app.init();

    while (!renderer.window().should_close() && !app.input().is_key_pressed(Key::Escape)) {
        glfwPollEvents();

        renderer.render();
    }
}
}
