#include <format>
#include <kata/resource/shader.hpp>

namespace kata {
Result<ShaderCompiler> ShaderCompiler::create()
{
    Slang::ComPtr<slang::IGlobalSession> global_session;
    createGlobalSession(global_session.writeRef());

    return ShaderCompiler(global_session);
}

Result<ShaderBytecode> ShaderCompiler::compile_module(std::string const& name, std::string const& entry_point)
{
    slang::TargetDesc target_desc {};
    target_desc.format = SLANG_SPIRV;
    target_desc.profile = m_global_session->findProfile("spirv_1_6");

    char const* search_paths[] = { "../resources/shader" };

    slang::SessionDesc session_desc;
    session_desc.targets = &target_desc;
    session_desc.targetCount = 1;
    session_desc.searchPaths = search_paths;
    session_desc.searchPathCount = 1;

    Slang::ComPtr<slang::ISession> session;
    m_global_session->createSession(session_desc, session.writeRef());

    Slang::ComPtr<slang::IBlob> diagnostics;
    slang::IModule* module = session->loadModule(name.c_str(), diagnostics.writeRef());

    if (diagnostics) {
        auto error_text = std::format("error while compiling shader module `{}`:\n{}",
            name, static_cast<char const*>(diagnostics->getBufferPointer()));

        return Error::with_message(error_text);
    }

    Slang::ComPtr<slang::IEntryPoint> module_entry_point;
    module->findEntryPointByName(entry_point.c_str(), module_entry_point.writeRef());

    if (!module_entry_point) {
        return Error::with_message(std::format("couldn't find entry point `{}` in `{}`", entry_point, name));
    }

    slang::IComponentType* components[] = { module, module_entry_point };
    Slang::ComPtr<slang::IComponentType> program;
    session->createCompositeComponentType(components, 2, program.writeRef());

    Slang::ComPtr<slang::IComponentType> linked_program;
    Slang::ComPtr<slang::IBlob> link_diagnostics;
    program->link(linked_program.writeRef(), link_diagnostics.writeRef());

    if (link_diagnostics) {
        auto error_text = std::format("error while linking shader program (for module `{}`):\n{}",
            name, static_cast<char const*>(link_diagnostics->getBufferPointer()));

        return Error::with_message(error_text);
    }

    int entry_point_index = 0; // only one entry point
    int target_index = 0;      // only one target
    Slang::ComPtr<slang::IBlob> kernel;
    Slang::ComPtr<slang::IBlob> kernel_diagnostics;
    linked_program->getEntryPointCode(
        entry_point_index,
        target_index,
        kernel.writeRef(),
        kernel_diagnostics.writeRef());

    if (kernel_diagnostics) {
        auto error_text = std::format("error while producing code for shader module `{}`:\n{}",
            name, static_cast<char const*>(kernel_diagnostics->getBufferPointer()));

        return Error::with_message(error_text);
    }

    auto bytecode_ptr = static_cast<uint8_t const*>(kernel->getBufferPointer());
    auto bytecode_size = kernel->getBufferSize();

    ShaderBytecode bytecode(bytecode_size);

    std::copy(bytecode_ptr, bytecode_ptr + bytecode_size, bytecode.data());

    return bytecode;
}
}
