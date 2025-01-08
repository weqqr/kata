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

void panic(Error error)
{
    auto const& location = error.location();

    spdlog::critical("fatal error: {}", error.text());
    spdlog::critical("  in {}", location.function_name());
    spdlog::critical("  -  at {}:{}:{}", location.file_name(), location.line(), location.column());

    std::exit(EXIT_FAILURE);
}
}
