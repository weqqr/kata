cmake_minimum_required(VERSION 3.20)
project(kata LANGUAGES C CXX)

#
# 3rd-party dependencies
#

# Volk
if (WIN32)
   set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
endif()

add_subdirectory(deps/volk)

# spdlog
set(SPDLOG_USE_STD_FORMAT ON CACHE BOOL "use c++20 std::format" FORCE)

add_subdirectory(deps/spdlog)

# Slang
set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/deps/slang/cmake")
include(slangTargets)

target_include_directories(slang::slang INTERFACE deps/slang/include)
configure_file(deps/slang/bin/slang.dll slang.dll COPYONLY)
configure_file(deps/slang/bin/slang-glslang.dll slang-glslang.dll COPYONLY)

# GLFW
add_subdirectory(deps/glfw)

#
# Engine library
#

add_library(kata STATIC
    kata/app/app.cpp
    kata/core/error.cpp
    kata/ecs/id_allocator.cpp
    kata/ecs/registry.cpp
    kata/ecs/system.cpp
    kata/input/input.cpp
    kata/render/render.cpp
    kata/render/window.cpp
    kata/resource/shader.cpp
    kata/rhi/command.cpp
    kata/rhi/context.cpp
)
target_compile_features(kata PUBLIC cxx_std_20)
target_link_libraries(kata glfw volk spdlog slang::slang)
target_include_directories(kata PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

#
# Game executable
#

add_executable(game
    game/main.cpp
)
target_compile_features(game PUBLIC cxx_std_20)
target_link_libraries(game kata spdlog)
target_include_directories(game PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
