#pragma once

#include <kata/core/error.hpp>
#include <slang-com-ptr.h>
#include <slang.h>
#include <string>
#include <vector>

namespace kata {
using ShaderBytecode = std::vector<uint8_t>;

class ShaderCompiler {
public:
    ShaderCompiler() = default;

    static Result<ShaderCompiler> create();

    Result<ShaderBytecode> compile_module(std::string const& name, std::string const& entry_point);

private:
    ShaderCompiler(
        Slang::ComPtr<slang::IGlobalSession> global_session)
        : m_global_session(global_session)
    {
    }

    Slang::ComPtr<slang::IGlobalSession> m_global_session {};
};
}
