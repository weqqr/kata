cmake_minimum_required(VERSION 3.20)
project(blaze LANGUAGES C CXX)

if (WIN32)
   set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
endif()

set(SPDLOG_USE_STD_FORMAT ON CACHE BOOL "use c++20 std::format" FORCE)

add_subdirectory(deps/spdlog)
add_subdirectory(deps/volk)
add_subdirectory(deps/glfw)

add_library(kata STATIC
    kata/app/app.cpp
    kata/core/error.cpp
    kata/ecs/id_allocator.cpp
    kata/ecs/registry.cpp
    kata/input/input.cpp
    kata/render/render.cpp
    kata/render/window.cpp
    kata/rhi/command.cpp
    kata/rhi/context.cpp
)
target_compile_features(kata PUBLIC cxx_std_20)
target_link_libraries(kata glfw volk spdlog)
target_include_directories(kata PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(game
    game/main.cpp
)
target_compile_features(game PUBLIC cxx_std_20)
target_link_libraries(game kata spdlog)
target_include_directories(game PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
