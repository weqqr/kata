#include <kata/core/error.hpp>
#include <spdlog/spdlog.h>
#include <utility>

namespace kata {
Error::Error(std::string text, std::source_location location)
    : m_text(text)
    , m_location(location)
    , m_is_nil(true)
{
}

Error Error::with_message(std::string text, std::source_location const location)
{
    return Error(std::move(text), location);
}

void panic(Error error, std::source_location const location)
{
    spdlog::critical("");
    spdlog::critical("-----");
    spdlog::critical("panic");
    spdlog::critical("-----");
    spdlog::critical("");
    spdlog::critical("Fatal error: {}", error.text());
    spdlog::critical("Caused by panic call:");
    spdlog::critical("  in function `{}`", location.function_name());
    spdlog::critical("  at {}:{}:{}", location.file_name(), location.line(), location.column());
    spdlog::critical("");
    spdlog::critical("Caused by error:");
    spdlog::critical("  originating in function `{}`", error.location().function_name());
    spdlog::critical("  at {}:{}:{}", error.location().file_name(), error.location().line(), error.location().column());

    std::exit(EXIT_FAILURE);
}
}
