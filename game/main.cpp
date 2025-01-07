#include <GLFW/glfw3.h>
#include <kata/app/app.hpp>
#include <kata/core/error.hpp>
#include <kata/ecs/registry.hpp>
#include <kata/render/render.hpp>
#include <kata/render/window.hpp>
#include <spdlog/spdlog.h>

namespace game {
class App : public kata::App {
    virtual void init() override
    {
        spdlog::info("hello, world!");
    }
};
}

int main()
{
    game::App app;
    kata::run(app);
}
