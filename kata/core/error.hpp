#pragma once

#include <source_location>
#include <string>
#include <utility>

#define OR_RETURN(err) \
    ;                  \
    if (err) {         \
        return err;    \
    }

namespace kata {
class Error {
public:
    Error() = default;

    static Error with_message(std::string text, std::source_location const location = std::source_location::current());

    explicit operator bool() const
    {
        return m_is_nil;
    }

    std::string const& text() const
    {
        return m_text;
    }

    std::source_location const& location() const
    {
        return m_location;
    }

    bool is_nil() const
    {
        return m_is_nil;
    }

private:
    explicit Error(std::string text, std::source_location location);

    std::string m_text {};
    std::source_location m_location {};
    bool m_is_nil { false }; // something zero-sized would be nicer
};

template<typename T, typename E = Error>
class Result {
public:
    Result(T value)
        : m_value(std::move(value))
    {
    }

    Result(E error)
        : m_error(error)
    {
    }

    explicit operator bool() const
    {
        return m_error.is_nil();
    }

    bool is_error() const
    {
        return m_error.is_nil();
    }

    E& error()
    {
        return m_error;
    }

    T& value()
    {
        return m_value;
    }

    T& operator*()
    {
        return m_value;
    }

    T release_value()
    {
        return std::move(m_value);
    }

    template<std::size_t Index>
    std::tuple_element_t<Index, Result>& get()
    {
        if constexpr (Index == 0) {
            return m_value;
        } else if constexpr (Index == 1) {
            return m_error;
        }
    }

private:
    T m_value {};
    Error m_error {};
};
}

namespace std {
template<typename T, typename E>
struct tuple_size<::kata::Result<T, E>> {
    static constexpr size_t value = 2;
};

template<typename T, typename E>
struct tuple_element<0, ::kata::Result<T, E>> {
    using type = T;
};

template<typename T, typename E>
struct tuple_element<1, ::kata::Result<T, E>> {
    using type = E;
};
}
