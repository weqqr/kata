#pragma once

#include <kata/core/error.hpp>
#include <slang-com-ptr.h>
#include <slang.h>
#include <string_view>

namespace kata {
class ShaderCompiler {
public:
    ShaderCompiler() = default;

    static Result<ShaderCompiler> create();

    void compile(std::string_view path);

private:
    ShaderCompiler(
        Slang::ComPtr<slang::IGlobalSession> globalSession)
        : m_globalSession(globalSession)
    {
    }

    Slang::ComPtr<slang::IGlobalSession> m_globalSession {};
};
}
