#include <utility>

#include <kata/core/error.hpp>

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
}
